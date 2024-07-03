#include "bit_buffer.h"
#include "gtest/gtest.h"

#include <random>
#include <memory>
#include <cstdio>

namespace
{

class bit_buffer_tester
{
public:
    bool compare() const
    {
        if( a.empty() != b.empty() ) {
            printf("a.empty() = %d, b.empty() = %d\n", a.empty(), b.empty());
            return false;
        }

        if( a.size() != b.size() ) {
            printf("a.size() = %ld, b.size() = %ld\n", a.size(), b.size());
            return false;
        }

        for( size_t i = 0; i < a.size(); i++ ) {
            if( a[i] != b.peek(i) ) {
                printf("a[i] = %d, b.peek(i) = %d\n", int(a[i]), int(b.peek(i)));
                return false;        
            }
        }

        return true;
    }

    void test( size_t N )
    {
        for( long i = 0; i < N; i++ ) {
            auto action = distrib( gen );    
            // 60% chance: push
            if( action < 60 ) {
                a.push_back( i % 2 );
                b.push_back( i % 2 );
            }
            // 25% chance: peek
            else if( action < 85 ) {
                if( a.empty() ) continue;
                auto idx = i % a.size();
                if( a[idx] != b.peek(idx) )
                    printf("a[idx] = %d, b.peek(idx) = %d\n", int(a[idx]), int(b.peek(idx)));
            }
            // 10% chance: populate from raw memory
            else if( action < 95 ) {
                if( b.empty() ) continue;
                const auto nbits  = b.size();
                const auto nbytes = b.data_size();
                auto tmp = std::make_unique<uint8_t[]>( nbytes );

                const uint8_t* data = b.data();
                for( size_t ii = 0; ii < nbytes; ii++ )
                    tmp[ii] = data[ii];

                // Test the status of b.populate(). If failure, then 
                //   trigger a comparison that's def. gonna fail.
                if( !b.populate( tmp.get(), nbytes, nbits ) ) {
                    b.clear();
                    break;
                }
            }
            // 3% chance: clear
            else if( action < 98 ) {
                a.clear();
                b.clear();
            }
            // 2% chance: access memory
            else {
                ptr = b.data();
            }
        }
    }

private:
    std::vector<bool> a;
    speck::bit_buffer b;
    const uint8_t* ptr = nullptr;

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen{rd()}; //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib{0, 99};
};


TEST( bit_buffer, hundreds )
{
    const size_t mult = 100;
    bit_buffer_tester tester;
    for( size_t i = 1; i < 10; i++ ) {
        tester.test( i * mult );
        EXPECT_EQ( tester.compare(), true );
    }
}


TEST( bit_buffer, thousands )
{
    const size_t mult = 1000;
    bit_buffer_tester tester;
    for( size_t i = 1; i < 10; i++ ) {
        tester.test( i * mult );
        EXPECT_EQ( tester.compare(), true );
    }
}


TEST( bit_buffer, many_operations )
{
    const size_t mult = 10000;
    bit_buffer_tester tester;
    for( size_t i = 1; i < 50; i++ ) {
        tester.test( i * mult );
        EXPECT_EQ( tester.compare(), true );
    }
}

}
