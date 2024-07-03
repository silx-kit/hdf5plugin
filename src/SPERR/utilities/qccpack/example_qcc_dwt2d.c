
#define HAVE_SPECK

#include "libQccPack.h"
#include "sam_helper.h"

#include <stdlib.h>


int array_to_pyramid( const float* array, QccWAVSubbandPyramid* pyramid )
{
    size_t counter = 0, row, col;
    for( row = 0; row < pyramid->num_rows; row++ )
        for( col = 0; col < pyramid->num_cols; col++ )
        {
            pyramid->matrix[row][col] = array[ counter++ ];
        }

    return 0;
}

int pyramid_to_array( const QccWAVSubbandPyramid* pyramid, double* array )
{
    size_t counter = 0, row, col;
    for( row = 0; row < pyramid->num_rows; row++ )
        for( col = 0; col < pyramid->num_cols; col++ )
        {
            array[counter++] = pyramid->matrix[row][col];
        }

    return 0;
}

int main( int argc, char* argv[] )
{
    if( argc != 5 )
    {
        printf("Usage: ./a.out input_filename dim_x dim_y output_filename\n");
        return 1;
    }

    const char*   input_name    = argv[1];
    const size_t  num_of_cols   = atoi( argv[2] );
    const size_t  num_of_rows   = atoi( argv[3] );
    const char*   output_name   = argv[4];
    const size_t  num_of_vals   = num_of_cols * num_of_rows;

    /* Read input data */
    float* in_buf = (float*)malloc( sizeof(float) * num_of_vals );
    if( sam_read_n_bytes( input_name, sizeof(float) * num_of_vals, in_buf ) != 0 )
    {
        printf("Error: read input file!\n");
        return 1;
    }

    /* Prepare Qcc data structure: QccWAVSubbandPyramid */
    QccWAVSubbandPyramid    pyramid;
    QccWAVSubbandPyramidInitialize( &pyramid );
    pyramid.num_levels = 0;
    pyramid.num_cols   = num_of_cols;
    pyramid.num_rows   = num_of_rows;
    if (QccWAVSubbandPyramidAlloc( &pyramid ))
    {    
        printf("(QccSPECKEncode): Error calling QccWAVSubbandPyramidAlloc()");
        return 1;
    } 
    array_to_pyramid( in_buf, &pyramid );

    /* Prepare Qcc data structure: QccWAVWavelet */
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
    
    /* Apply dwt */
    double image_mean;
    QccWAVSubbandPyramidSubtractMean( &pyramid, &image_mean, NULL );
    float min_xy = (float)num_of_cols;
    if( num_of_rows < num_of_cols )
          min_xy = (float)num_of_rows;
    float f      = log2f( min_xy / 8.0f );
    int num_level_xy = f < 0.0f ? 0 : (int)f + 1;
    QccWAVSubbandPyramidDWT( &pyramid, num_level_xy, &Wavelet );

    /* Apply idwt */
    QccWAVSubbandPyramidInverseDWT( &pyramid, &Wavelet );
    QccWAVSubbandPyramidAddMean( &pyramid, image_mean );

    /* write coefficients to a file */
    printf("mean = %lf\n", image_mean );
    double* out_buf = (double*)malloc( sizeof(double) * num_of_vals );
    pyramid_to_array( &pyramid, out_buf );
    if( sam_write_n_bytes( output_name, sizeof(double) * num_of_vals, out_buf ) )
    {
        printf("Output write error!\n");
        return 1;
    }

    /* compare the results against the original in double precision */
    double* in_bufd = (double*)malloc( sizeof(double) * num_of_vals );
    for( size_t i = 0; i < num_of_vals; i++ )
        in_bufd[i] = in_buf[i];
    double rmse, lmax, psnr, arr1min, arr1max;
    sam_get_statsd( in_bufd, out_buf, num_of_vals, 
                    &rmse, &lmax, &psnr, &arr1min, &arr1max );
    printf("Qcc: rmse = %f, lmax = %f, psnr = %fdB, orig_min = %f, orig_max = %f\n", 
            rmse, lmax, psnr, arr1min, arr1max );

    /* clean up */
    free( in_bufd );
    free( out_buf );
    QccWAVSubbandPyramidFree( &pyramid );
    free( in_buf );
}
