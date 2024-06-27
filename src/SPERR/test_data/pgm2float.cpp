#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <memory>

int main()
{
    // These values are specific to the lena image
    const char* infilename   = "lena80.pgm";
    const char* outfilename  = "lena80.float";
    const size_t total_size  = 6434;  // Find this value from the disk
    const size_t body_size   = 6400;  // Calculate this value based on the num. of pixels
    const size_t header_size   = total_size - body_size;

    std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>( total_size );
    std::ifstream infile( infilename, std::ios::binary );
    infile.read( reinterpret_cast<char*>(buf.get()), total_size );
    infile.close();

    std::vector<float> outbuf( body_size );
    for( int i = 0; i < body_size; i++ )
        outbuf[i] = buf[ i + header_size ];

    std::ofstream outfile( outfilename, std::ios::binary );
    outfile.write( reinterpret_cast<char*>( outbuf.data() ), sizeof(float) * body_size );
    outfile.close();
}
