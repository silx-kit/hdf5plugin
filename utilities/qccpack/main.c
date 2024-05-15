#define HAVE_SPECK

#include "libQccPack.h"
#include <stdio.h>
#include <stdlib.h>

/* This definitions ain't in libQccPack.h ... */
typedef struct
{
  char type;
  int level;
  int origin_row;
  int origin_col;
  int num_rows;
  int num_cols;
  char significance;
} QccSPECKSet;
#define QCCSPECK_SET_S 0
#define QCCSPECK_SET_I 1

static int QccSPECKSetSize(QccSPECKSet *set,
                           const QccWAVSubbandPyramid *coefficients,
                           int subband) /* specifies which chunk (0, 1, 2, 3), though 
                                           its mechanism seems quite complex. */
{                                       
  if (QccWAVSubbandPyramidSubbandOffsets(coefficients,      /* input */
                                         subband,           /* input */
                                         &set->origin_row,  /* output */
                                         &set->origin_col)) /* output */
    {     
      QccErrorAddMessage("(QccSPECKSetSize): Error calling QccWAVSubbandPyramidSubbandOffsets()");
      return(1);                        
    }                                   
  if (QccWAVSubbandPyramidSubbandSize(coefficients,         /* input */  
                                      subband,              /* input */
                                      &set->num_rows,       /* output */
                                      &set->num_cols))      /* output */
    {                                   
      QccErrorAddMessage("(QccSPECKSetSize): Error calling QccWAVSubbandPyramidSubbandSize()");
      return(1);                        
    }   
      
  return(0);
} 



void print_size( const QccSPECKSet* set )
{
    printf("    start position: %d,  %d\n", set->origin_col, set->origin_row );
    printf("    length        : %d,  %d\n", set->num_cols,   set->num_rows   );
}

int main( int argc, char* argv[] )
{
    if( argc != 4 )
    {
        printf("Usage: ./a.out dim_x, dim_y, num_of_levels\n");
        return 1;
    }

    const long dim_x   = atol( argv[1] );
    const long dim_y   = atol( argv[2] );
    const long num_lev = atol( argv[3] );


    /* Create a subband pyramid */
    QccWAVSubbandPyramid    pyramid;
    QccWAVSubbandPyramidInitialize( &pyramid );
    pyramid.num_levels = num_lev;
    pyramid.num_cols   = dim_x;
    pyramid.num_rows   = dim_y;
    if (QccWAVSubbandPyramidAlloc( &pyramid ))
    {    
        printf("(QccSPECKEncode): Error calling QccWAVSubbandPyramidAlloc()");
        return 1;
    } 

    /* Create a QccSPECKSet */
    QccSPECKSet  set;
    set.type  = QCCSPECK_SET_S;
    set.level = num_lev;

    for( int i = 0; i < 4; i++ )
    {
        QccSPECKSetSize( &set, &pyramid, i );
        printf("subband = %d\n", i );
        print_size( &set );
    }


    QccWAVSubbandPyramidFree( &pyramid );
}
