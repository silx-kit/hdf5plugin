/* This program converts raw data values
 * into a QccIMGImageCube object, then write this object
 * to an ICB-format file, using the QccIMGImageCube function.
 *
 * Programmer: Samuel Li
 * Date: 7/2/2015
 *
 * Modified: 8/5/2015
 *      Read double precision added.
 */

#include "libQccPack.h"

#ifdef DOUBLE
#define FLOAT_SIZE 8
#else
#define FLOAT_SIZE 4
#endif

#define USG_STRING "%d:dim_x %d:dim_y %d:dim_z %s:input_name (raw) %s:output_name (icb)"


int ImageCubeReadData( QccString filename, QccIMGImageCube* image_cube )
{
    FILE* infile = fopen( filename, "rb" );
    if( infile  == NULL ) {
        printf( "Read file open error!\n" );
        exit(1);
    }

    fseek( infile, 0, SEEK_END );
    long size = ftell( infile );
    if( size % FLOAT_SIZE == 0 ) {
        size /= FLOAT_SIZE;
        if( size != (image_cube->num_cols) * (image_cube->num_rows) *
                    (image_cube -> num_frames)  )   {
            printf( "Read file length error!\n" );
            exit(1);
        }
    }
    else{
        printf( "Read file broken!\n" );
        exit(1);
    }

    double min = MAXDOUBLE;
    double max = -MAXDOUBLE;
    int frame, row, col;
    long planeSize = (image_cube -> num_cols) * (image_cube -> num_rows);
    void* ptr = malloc( FLOAT_SIZE * planeSize );
#ifdef DOUBLE
    double* buf = (double*) ptr;
#else
    float* buf = (float*) ptr; 
#endif

    for( frame = 0; frame < image_cube -> num_frames; frame++ ) {
        fseek( infile, FLOAT_SIZE * frame * planeSize, SEEK_SET );
        long result = fread( buf, FLOAT_SIZE, planeSize, infile );
        if( result != planeSize ) {
            printf( "Input file read error!\n");
            exit(1);
        }
        long i;
        for( i = 0; i < planeSize; i++ ) {
            if( buf[i] < min )      min = buf[i];
            if( buf[i] > max )      max = buf[i];
        }
        i = 0;
        for (row = 0; row < image_cube->num_rows; row++)
            for (col = 0; col < image_cube->num_cols; col++)
                image_cube -> volume[frame][row][col] = buf[ i++ ]; 
    }

    image_cube -> min_val = min;
    image_cube -> max_val = max;
    free( ptr );
    return 0;
}

int main (int argc, char* argv[] )
{
    QccIMGImageCube imagecube;
    QccIMGImageCubeInitialize( &imagecube );
    QccString input_name;    

    if( QccParseParameters( argc, argv, USG_STRING, &imagecube.num_cols, &imagecube.num_rows,
                            &imagecube.num_frames, input_name, imagecube.filename ) )
        QccErrorExit();
 

    if( QccIMGImageCubeAlloc( &imagecube ) )
        QccErrorPrintMessages();


    ImageCubeReadData( input_name, &imagecube );
    QccIMGImageCubeWrite( &imagecube );

    //QccIMGImageCubePrint( &imagecube );

    QccIMGImageCubeFree( &imagecube );

    return 0;
}
