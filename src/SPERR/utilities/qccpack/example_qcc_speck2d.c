/* 
 * This program reads a 2D binary file, utilizes SPECK encoder
 * to compress to a target bitrate, and then reports the compression errors.
 */

#define HAVE_SPECK

#include <sys/time.h>

#include "libQccPack.h"
#include "sam_helper.h"
#include "assert.h"

#include "SpeckConfig.h"


int array_to_image( const float* array, QccIMGImageComponent* image )
{
    float min = array[0], max = array[0];
    size_t counter = 0, row, col;
    for( row = 0; row < image->num_rows; row++ )
        for( col = 0; col < image->num_cols; col++ )
        {
            float val = array[ counter++ ];
            image->image[row][col] = val;
            if( val < min )     min = val;
            if( val > max )     max = val;
        }

    image->min_val = min;
    image->max_val = max;

    return 0;
}

int image_to_array( const QccIMGImageComponent* image, float* array )
{
    size_t counter = 0, row, col;
    for( row = 0; row < image->num_rows; row++ )
        for( col = 0; col < image->num_cols; col++ )
        {
            array[counter++] = image->image[row][col];
        }

    return 0;
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
    if( argc != 5 )
    {
        fprintf( stderr, "Usage: ./a.out input_raw_file dim_X, dim_Y, comp_ratio\n" );
        return 1;
    }
    const char* input_name    = argv[1];
    const char* output_name   = "qcc.bitstream";
    int         num_of_cols   = atoi( argv[2] );
    int         num_of_rows   = atoi( argv[3] );
    const size_t num_of_vals  = num_of_cols * num_of_rows;
    const size_t num_of_bytes = sizeof(float) * num_of_vals;
    int         num_of_levels = calc_num_of_xforms( num_of_cols );

    const float cratio        = atof( argv[4] );
    const int   header_size   = 21;                         /* bytes */
    /* Allocate bit budget for the header too */
    const int   total_bits    = (int)(8.0f * num_of_bytes / cratio) + 8 * header_size;

    /*
     * Stage 1: Encoding
     */

    /* Prepare necessary Wavelets. */
    QccString             WaveletFilename = QCCWAVWAVELET_DEFAULT_WAVELET;
    QccString             Boundary = "symmetric";
    QccWAVWavelet         Wavelet;
    if( QccWAVWaveletInitialize( &Wavelet ) ) 
    {
        fprintf( stderr, "QccWAVWaveletInitialize failed.\n" );
        return 1;
    }
    if( QccWAVWaveletCreate( &Wavelet, WaveletFilename, Boundary ) )
    {
        fprintf( stderr, "QccWAVWaveletCreate failed.\n" );
        return 1;
    }

    /* Prepare necessary output buffer. */
    QccBitBuffer          OutputBuffer;
    if( QccBitBufferInitialize( &OutputBuffer ) )
    {
        fprintf( stderr, "QccBitBufferInitialize failed.\n" );
        return 1;
    }
    QccStringCopy( OutputBuffer.filename, output_name );
    OutputBuffer.type     = QCCBITBUFFER_OUTPUT;
    if (QccBitBufferStart(&OutputBuffer))
    {
        fprintf( stderr, "QccBitBufferStart failed.\n" );
        return 1;
    }

    /* Prepare necessary image component. */
    QccIMGImageComponent    Image;
    if( QccIMGImageComponentInitialize( &Image ) )
    {
        fprintf( stderr, "QccIMGImageComponentInitialize failed.\n" );
        return 1;
    }
    QccStringCopy( Image.filename, input_name );
    Image.num_cols        = num_of_cols;
    Image.num_rows        = num_of_rows;
    if( QccIMGImageComponentAlloc( &Image ) )
    {
        fprintf( stderr, "QccIMGImageComponentAlloc failed.\n" );
        return 1;
    }

    /* Get ready to read data. */
    float* in_array = (float*)malloc( num_of_bytes );
    if( sam_read_n_bytes( input_name, num_of_bytes, in_array ) != 0 )
        return 1;
    if( array_to_image( in_array, &Image ) )
    {
        fprintf( stderr, "sam_read_data failed.\n" );
        free( in_array );
        return 1;
    }

    /* Encode to a bitstream, and write to the bit stream. */
    struct timeval start, end;
    gettimeofday( &start, NULL );
    if( QccSPECKEncode( &Image, NULL, num_of_levels, 
                        total_bits, &Wavelet, &OutputBuffer ) )
    {
        fprintf( stderr, "QccSPECKEncode failed.\n" );
        free( in_array );
        return 1;
    }
    if( QccBitBufferEnd( &OutputBuffer ) )
    {
        fprintf( stderr, "QccBitBufferEnd failed.\n" );
        free( in_array );
        return 1;
    }


    /*
     * Stage 2: Decoding
     */

    /* Prepare input buffer. */
    QccBitBuffer                 InputBuffer;
    if( QccBitBufferInitialize( &InputBuffer ) )
    {
        fprintf( stderr, "QccBitBufferInitialize failed.\n" );
        free( in_array );
        return 1;
    }
    QccStringCopy( InputBuffer.filename, output_name );
    InputBuffer.type = QCCBITBUFFER_INPUT;
    if (QccBitBufferStart(&InputBuffer))
    {
        fprintf( stderr, "QccBitBufferStart failed.\n" );
        free( in_array );
        return 1;
    }

    double image_mean;
    int    max_coeff_bits;
    if( QccSPECKDecodeHeader( &InputBuffer, &num_of_levels, &num_of_rows, 
                              &num_of_cols, &image_mean, &max_coeff_bits ) )
    {
        fprintf( stderr, "QccSPECKDecodeHeader failed.\n" );
        free( in_array );
        return 1;
    }
    assert( num_of_vals == num_of_rows * num_of_cols );

    /* Note that we re-use the ImageComponent that was used for encoding. */

    if( QccSPECKDecode( &InputBuffer, &Image, NULL, num_of_levels, &Wavelet,
                        image_mean, max_coeff_bits, total_bits           ) )
    {
        fprintf( stderr, "QccSPECKDecode failed.\n" );
        free( in_array );
        return 1;
    }
    gettimeofday( &end, NULL );
    long elapsed = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    printf("qcc takes milliseconds: %f\n", (float)elapsed / 1000.0f );

    /* Evaluate the encoding and decoding process! */
    float* out_array = (float*)malloc( num_of_bytes );
    image_to_array( &Image, out_array );
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


    /* Cleanup */
    QccWAVWaveletFree(&Wavelet);
    QccIMGImageComponentFree( &Image );
    free( in_array );
    free( out_array );

    return 0;
}
