/* 
 * This program reads a 3D binary file, utilizes SPECK encoder
 * to compress to a target bitrate, and then reports the compression errors.
 */

#define HAVE_SPECK

#include "libQccPack.h"
#include "sam_helper.h"
#include "assert.h"

#include "SpeckConfig.h"

#include <sys/time.h>


void array_to_image_cube( const float* array, QccIMGImageCube* imagecube )
{
    float min = array[0];
    float max = array[0];
    size_t idx = 0, frame, row, col;
    for( frame = 0; frame < imagecube -> num_frames; frame++ )
        for( row = 0; row < imagecube -> num_rows; row++ )
            for( col = 0; col < imagecube -> num_cols; col++ )
            {
                if( array[idx] < min )         min = array[idx];
                if( array[idx] > max )         max = array[idx];
                imagecube -> volume[frame][row][col] = array[idx];
                idx++;
            }
    imagecube -> min_val = min;
    imagecube -> max_val = max;
}


void image_cube_to_array( const QccIMGImageCube* imagecube, float* array )
{
    size_t idx = 0, frame, row, col;
    for( frame = 0; frame < imagecube -> num_frames; frame++ )
        for( row = 0; row < imagecube -> num_rows; row++ )
            for( col = 0; col < imagecube -> num_cols; col++ )
            {
                array[idx++] = imagecube -> volume[frame][row][col];
            }
}


int calc_num_of_xforms( int len )
{
    assert( len > 0 );
    // I decide 8 is the minimal length to do one level of xform.
    float ratio = (float)len / 8.0f;
    float f     = logf(ratio) / logf(2.0f);
    int num_of_xforms = f < 0.0f ? 0 : (int)f + 1;

    return num_of_xforms;
}


int main( int argc, char** argv )
{
    if( argc != 6 )
    {
        fprintf( stderr, "Usage: ./a.out input_file dim_x dim_y dim_z cratio\n" );
        return 1;
    }
    const char* input_name    = argv[1];
    int         num_of_cols   = atoi( argv[2] );
    int         num_of_rows   = atoi( argv[3] );
    int         num_of_frames = atoi( argv[4] );
    double      cratio        = atof( argv[5] );
    const size_t num_of_vals  = num_of_cols * num_of_rows * num_of_frames;
    const size_t num_of_bytes = sizeof(float) * num_of_vals;

    const char* output_name   = "qcc.tmp";
    const int header_size     = 29;
    const int total_bits      = (int)(8.0f * num_of_bytes / cratio) + header_size * 8;

    /* 
     * Stage 1: Encoding
     */

    /* Prepare wavelet */
	QccString       waveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
    QccString       boundary = "symmetric";
    QccWAVWavelet   wavelet;
    if( QccWAVWaveletInitialize(&wavelet) )
    {
        fprintf( stderr, "QccWAVWaveletInitialize failed.\n" );
        return 1;
    }
    if (QccWAVWaveletCreate( &wavelet, waveletFilename, boundary )) 
    {
        fprintf( stderr, "Error calling QccWAVWaveletCreate()\n" );
        return 1;
    }    

    /* Prepare output bit buffer */
    QccBitBuffer    output_buffer;
    if( QccBitBufferInitialize(&output_buffer) )
    {
        fprintf( stderr, "QccBitBufferInitialize() failed\n" ); return 1;
    }    
    QccStringCopy( output_buffer.filename, output_name );
    output_buffer.type = QCCBITBUFFER_OUTPUT;
    if (QccBitBufferStart(&output_buffer))
    {
        fprintf( stderr, "Error calling QccBitBufferStart()\n");
        return 1;
    }

    /* Prepare image cube */
    float* in_array = (float*)malloc( num_of_bytes );
    if( sam_read_n_bytes( input_name, num_of_bytes, in_array ) )
    {
        fprintf( stderr, "file read error: %s\n", input_name );
        return 1;
    }
    QccIMGImageCube     imagecube;
    if( QccIMGImageCubeInitialize( &imagecube ) )
    {
        fprintf( stderr, "QccIMGImageCubeInitialize() failed\n");
        return 1;
    }
    imagecube.num_cols = num_of_cols;
    imagecube.num_rows = num_of_rows;
    imagecube.num_frames = num_of_frames;
    if( QccIMGImageCubeAlloc( &imagecube ) )
    {
        fprintf( stderr, "QccIMGImageCubeAlloc() failed\n");
        return 1;
    }
    array_to_image_cube( in_array, &imagecube );

    /* Get ready for SPECK! */
    int transform_type   = QCCWAVSUBBANDPYRAMID3D_PACKET;
    int num_of_levels_xy = calc_num_of_xforms( num_of_cols );
    int num_of_levels_z  = calc_num_of_xforms( num_of_frames );
    struct timeval start, end;
    gettimeofday( &start, NULL );
    if( QccSPECK3DEncode( &imagecube, NULL, transform_type,
                          num_of_levels_z, num_of_levels_xy,
                          &wavelet, &output_buffer, total_bits ) )
    {
        fprintf( stderr, "QccSPECK3DEncode() failed.\n" );
        return 1;
    }

    /* Write bit buffer to disk, and free resources */
    if( QccBitBufferEnd( &output_buffer ) )
    {
        fprintf( stderr, "QccBitBufferEnd() failed.\n" );
        return 1;
    }


    /*
     * Now start the decode procedures.
     */

    /* Prepare an input buffer */
    QccBitBuffer    input_buffer;
    QccBitBufferInitialize( &input_buffer );
    QccStringCopy( input_buffer.filename, output_name );
    input_buffer.type = QCCBITBUFFER_INPUT;
    if (QccBitBufferStart(&input_buffer) ) 
    {
        fprintf( stderr, "Error calling QccBitBufferStart()\n" );
        return 1;
    }

    /* Decode header */
    double image_mean;
    int max_coeff_bits;
    if (QccSPECK3DDecodeHeader( &input_buffer,      &transform_type,    &num_of_levels_z, 
                                &num_of_levels_xy,  &num_of_frames,     &num_of_rows, 
                                &num_of_cols,       &image_mean,        &max_coeff_bits ))
    {
        fprintf( stderr, "Error calling QccSPECK3DDecodeHeader()\n");
        return 1;
    }

    /* Do a SPECK decode. Note that we reuse the imagecube object */
    int TargetBitCnt = QCCENT_ANYNUMBITS;
    if (QccSPECK3DDecode( &input_buffer,    &imagecube,         NULL, 
                          transform_type,   num_of_levels_z,    num_of_levels_xy,
                          &wavelet,         image_mean,         max_coeff_bits,
                          TargetBitCnt ))
    {
        QccErrorAddMessage("Error calling QccSPECK3DDecode()" );
        QccErrorExit();
    }
    if (QccBitBufferEnd(&input_buffer) ) 
    {
        fprintf( stderr, "Error calling QccBitBufferEnd()\n" );
        return 1;
    }
    gettimeofday( &end, NULL );
    long elapsed = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("qcc takes milliseconds: %f\n", (float)elapsed / 1000.0f );

    /* Collected the decoded array, and print out some statistics. */
    float* out_array = (float*)malloc( num_of_bytes );
    image_cube_to_array( &imagecube, out_array ); 
    float  rmse, lmax, min, max, psnr;
    if( sam_get_statsf( in_array, out_array, num_of_vals, &rmse, &lmax, &psnr, &min, &max ) )
    {
        fprintf( stderr, "get_rmse_max failed.\n" );
        free( in_array );
        free( out_array );
        return 1;
    }
    printf("rmse = %f, lmax = %f, psnr = %fdB, orig_min = %f, orig_max = %f\n",
            rmse, lmax, psnr, min, max );

    /* cleanup */
    QccWAVWaveletFree( &wavelet );
    QccIMGImageCubeFree( &imagecube );
    free( in_array );
    free( out_array );

    return 0;
}





