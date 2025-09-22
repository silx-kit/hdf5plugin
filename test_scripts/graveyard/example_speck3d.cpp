#include "SPECK3D.h"
#include "CDF97.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <chrono>


// #define FIRST_STEP


extern "C"  // C Function calls, and don't include the C header!
{
    int sam_read_n_bytes( const char*, size_t, void* );
    int sam_write_n_doubles( const char*, size_t, const double* );
    int sam_get_statsd( const double* arr1, const double* arr2, size_t len,
                        double* rmse,       double* lmax,   double* psnr,
                        double* arr1min,    double* arr1max            );
}


int main( int argc, char* argv[] )
{

    if( argc != 6 )
    {

#ifdef QZ_TERM
        std::cerr << "Usage: ./a.out input_filename dim_x dim_y dim_z qz_levels" << std::endl;
#else
        std::cerr << "Usage: ./a.out input_filename dim_x dim_y dim_z cratio" << std::endl;
#endif

        return 1;
    }

    char output[256];
    std::strcpy( output, argv[0] );
    std::strcat( output, ".tmp" );
    
    const char*   input  = argv[1];
    const size_t  dim_x  = std::atol( argv[2] );
    const size_t  dim_y  = std::atol( argv[3] );
    const size_t  dim_z  = std::atol( argv[4] );
    const size_t  total_vals = dim_x * dim_y * dim_z;

#ifdef QZ_TERM
    const int     qz_levels = std::atoi( argv[5] );
#else
    const float   cratio = std::atof( argv[5] );
#endif

#ifdef NO_CPP14
    speck::buffer_type_f in_buf( new float[total_vals] );
#else
    speck::buffer_type_f in_buf = std::make_unique<float[]>( total_vals );
#endif

    // Let's read in binaries as 4-byte floats
    if( sam_read_n_bytes( input, sizeof(float) * total_vals, in_buf.get() ) )
    {
        std::cerr << "Input read error!" << std::endl;
        return 1;
    }

    // Take input to go through DWT.
    speck::CDF97 cdf;
    cdf.set_dims( dim_x, dim_y, dim_z );
    cdf.copy_data( in_buf, total_vals );
    const auto startT = std::chrono::high_resolution_clock::now();
    cdf.dwt3d();

    // Do a speck encoding
    speck::SPECK3D encoder;
    encoder.set_dims( dim_x, dim_y, dim_z );
    encoder.set_image_mean( cdf.get_mean() );
    encoder.take_coeffs( cdf.release_data(), total_vals );

#ifdef QZ_TERM
    #ifdef FIRST_STEP
        encoder.set_quantization_iterations( qz_levels );
    #else
        encoder.set_quantization_term_level( qz_levels );
    #endif
#else
    const size_t total_bits = size_t(32.0f * total_vals / cratio);
    encoder.set_bit_budget( total_bits );
#endif

    encoder.encode();
    encoder.write_to_disk( output );

    // Do a speck decoding
    speck::SPECK3D  decoder;
    decoder.read_from_disk( output );

#ifdef QZ_TERM
    decoder.set_bit_budget( 0 );
#else
    decoder.set_bit_budget( total_bits );
#endif

    decoder.decode();

    // Do an inverse wavelet transform
    speck::CDF97 idwt;
    size_t dim_x_r, dim_y_r, dim_z_r;
    decoder.get_dims( dim_x_r, dim_y_r, dim_z_r );
    idwt.set_dims( dim_x_r, dim_y_r, dim_z_r );
    idwt.set_mean( decoder.get_image_mean() );
    idwt.take_data( decoder.release_coeffs_double(), dim_x_r * dim_y_r * dim_z_r );
    idwt.idwt3d();

    // Finish timer and print timing
    const auto endT   = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> diffT  = endT - startT;
    //std::cout << "Time for SPECK in milliseconds: " << diffT.count() * 1000.0f << std::endl;

    // Compare the result with the original input in double precision

#ifdef QZ_TERM
    float bpp = float(encoder.get_num_of_bits()) / float(total_vals);
#endif


#ifdef FIRST_STEP
    speck::buffer_type_d in_bufd = std::make_unique<double[]>( total_vals );
    for( size_t i = 0; i < total_vals; i++ )
        in_bufd[i] = in_buf[i];

    double rmse, lmax, psnr, arr1min, arr1max;
    sam_get_statsd( in_bufd.get(), idwt.get_read_only_data().get(), 
                    total_vals, &rmse, &lmax, &psnr, &arr1min, &arr1max );
    printf("Sam stats: rmse = %f, lmax = %f, psnr = %fdB, orig_min = %f, orig_max = %f\n", 
            rmse, lmax, psnr, arr1min, arr1max );
    in_bufd.reset( nullptr );

    printf("With %d levels of quantization, average BPP = %f, and qz terminates at level %d\n",
            qz_levels, bpp, encoder.get_quantization_term_level() );

    // For the first step, print out the error values at the following positions:
    // 0, 50, 100, 200, 300, 400, 500, 1000

    std::vector<float> diff;
    diff.reserve( total_vals );
    for( size_t i = 0; i < total_vals; i++ )
        diff.push_back( std::abs(in_buf[i] - float(idwt.get_read_only_data()[i]) ));
    std::partial_sort( diff.begin(), diff.begin() + 1001, diff.end(), std::greater<float>() );
    std::array<size_t, 8> idx { 0, 50, 100, 200, 300, 400, 500, 1000 };
    for( auto i : idx )
        std::cout << diff[i] << ",  ";
    std::cout << std::endl;

#else 

    // Count how many data points end up having errors greater than the thresholds.
    //
    const size_t num_of_th = 8;
    //  Variable dbz, qz_levels = 17
    //std::array<float, num_of_th> threshold{0.734512,  0.596821,  0.56641,  0.535465,  
    //                                       0.519047,  0.504593,  0.491455,  0.459518 };
    //  Variable hvort, qz_levels = 14  -9
    //std::array<float, num_of_th> threshold{0.00615794,  0.00417704,  0.00400225,  0.00382341,  
    //                                       0.00369853,  0.00361842,  0.00356053,  0.00338412 };
    //  Variable prespert, qz_levels = 20  -7
    //std::array<float, num_of_th> threshold{0.0246198,  0.0170196,  0.01632,  0.0154797,  
    //                                       0.0149632,  0.0146113,  0.0143417,  0.0134916};
    //  Variable qc, qz_levels = 20
    //std::array<float, num_of_th> threshold{0.010118,  0.00804029,  0.00763616,  0.00728243,  
    //                                       0.00708556,  0.00691819,  0.00680445,  0.00638294};
    //  Variable qg, qz_levels = 23  -11
    //std::array<float, num_of_th> threshold{0.00146534,  0.0010819,  0.00103881,  0.000984068,  
    //                                       0.000951942,  0.000932336,  0.000913382,  0.000861526};
    //  Variable qr, qz_levels = 20  -9
    //std::array<float, num_of_th> threshold{0.00518513,  0.00401469,  0.00384271,  0.00364681,  
    //                                       0.00351448,  0.00343102,  0.00336915,  0.0031701};
    //  Variable streamvort, qz_levels = 14  -9
    //std::array<float, num_of_th> threshold{0.00578914,  0.00446857,  0.00430652,  0.00411505,  
    //                                       0.00397266,  0.00388597,  0.00380701,  0.00359325};
    //  Variable thrhopert, qz_levels = 18  -6
    std::array<float, num_of_th> threshold{0.0559266,  0.0360634,  0.0344715,  0.03269,  
                                           0.0315752,  0.0308189,  0.0302553,  0.0283475 };
    //  Variable uinterp, qz_levels = 19  -5
    //std::array<float, num_of_th> threshold{0.0894642,  0.0709276,  0.0674381,  0.0647449,  
    //                                       0.0627775,  0.0612926,  0.0602803,  0.0566506};
    //  Variable vortmag, qz_levels = 15  -9
    //std::array<float, num_of_th> threshold{0.00588624,  0.00424516,  0.00401397,  0.00382658,  
    //                                       0.0037123,  0.00362666,  0.00357628,  0.00338255};

    std::vector<float> diff;
    diff.reserve( total_vals );
    for( size_t i = 0; i < total_vals; i++ )
        diff.push_back( std::abs(in_buf[i] - float(idwt.get_read_only_data()[i]) ));

    std::array<size_t, num_of_th> count;
    count.fill( 0 );
    for( auto& e : diff ) {
        for( size_t i = 0; i < num_of_th; i++ ) {
            if( e > threshold[i] ) {
                while( i < num_of_th ) {
                    count[i]++;
                    i++;
                }
            }
        }
    }

    // First print out the effective bpp, then print out the number of outliers
    std::cout << bpp << ", ";
    for( auto& e : count )
        std::cout << e << ", ";
    std::cout << std::endl;
#endif



#ifdef EXPERIMENT
    // Experiment 1: 
    // Sort the differences and then write a tenth of it to disk.
    std::vector<speck::Outlier> LOS( total_vals, speck::Outlier{} );
    for( size_t i = 0; i < total_vals; i++ ) {
        LOS[i].location  = i;
        LOS[i].error = in_buf[i] - float(idwt.get_read_only_data()[i]);
    }
    
    const size_t num_of_outliers = total_vals / 10;
    std::partial_sort( LOS.begin(), LOS.begin() + num_of_outliers, LOS.end(), 
        [](auto& a, auto& b) { return (std::abs(a.error) > std::abs(b.error)); } );

    for( size_t i = 0; i < 10; i++ )
        printf("outliers: (%ld, %f)\n", LOS[i].location, LOS[i].error );

    std::ofstream file( "top_outliers", std::ios::binary );
    if( file.is_open() ) {
        file.write( reinterpret_cast<char*>(LOS.data()), sizeof(speck::Outlier) * num_of_outliers );
        file.close();
    }
#endif

}
