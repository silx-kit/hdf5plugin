#include "gtest/gtest.h"
#include <vector>
#include <limits>
#include <memory>

#include "compactor.h"

namespace {

TEST(compactor, strategy) {
  //
  // This test assumes that the compactor uses 32-bit integers.
  //
  // Create an array of all zeros
  size_t N = 32;
  auto buf = std::vector<unsigned int>(N, 0); 
  auto ans = compactor_strategy(buf.data(), N * sizeof(unsigned int)); 
  EXPECT_EQ(ans, 0);

  // Make the array of all ones 
  buf.assign(N, std::numeric_limits<unsigned int>::max());
  ans = compactor_strategy(buf.data(), N * sizeof(unsigned int)); 
  EXPECT_EQ(ans, 1);

  // Make the array half half
  for (int i = 0; i < N / 2; i++)
    buf[i] = 0;
  ans = compactor_strategy(buf.data(), N * sizeof(unsigned int)); 
  EXPECT_EQ(ans, 0);
}

TEST(compactor, comp_size) {
  //
  // This test assumes that the compactor uses 32-bit integers.
  //
  // Create an array of all zeros
  size_t N = 32;
  auto buf = std::vector<unsigned int>(N, 0);
  auto ans = compactor_comp_size(buf.data(), N * sizeof(unsigned int));
  EXPECT_EQ(ans, 9);

  // Make the array of all ones
  buf.assign(N, std::numeric_limits<unsigned int>::max());
  ans = compactor_comp_size(buf.data(), N * sizeof(unsigned int));
  EXPECT_EQ(ans, 9);

  // Make the array half half
  for (int i = 0; i < N / 2; i++)
    buf[i] = 0;
  ans = compactor_comp_size(buf.data(), N * sizeof(unsigned int));
  EXPECT_EQ(ans, 11);

  // Make the array 24 0's, 8 1's.
  for (int i = 0; i < N / 4; i++)
    buf[i] = buf[N - 1];
  ans = compactor_comp_size(buf.data(), N * sizeof(unsigned int));
  EXPECT_EQ(ans, 10);

  // Append the array with ints that need to be verbosely encoded.
  buf.push_back(1);
  buf.push_back(2);
  ans = compactor_comp_size(buf.data(), buf.size() * sizeof(unsigned int));
  EXPECT_EQ(ans, 18);
}

TEST(compactor, coding_all0_all1)
{
  // an array of all 0's
  int nbytes = 128;  // 32 ints
  auto buf = std::vector<unsigned char>(nbytes, 0);

  // encode `buf`
  auto encode = std::make_unique<unsigned char[]>(nbytes);
  auto encode_len = compactor_encode(buf.data(), nbytes, encode.get(), nbytes);
  EXPECT_EQ(encode_len, compactor_comp_size(buf.data(), nbytes));

  // decode and test all 0's
  auto decode = std::make_unique<unsigned char[]>(nbytes);
  auto decode_len = compactor_decode(encode.get(), nbytes, decode.get());
  EXPECT_EQ(decode_len, nbytes);
  for (int i = 0; i < nbytes; i++)
    ASSERT_EQ(decode[i], 0) << "i = " << i;

  // assign the entire array to be all 1's, and test again
  buf.assign(nbytes, 255);
  encode_len = compactor_encode(buf.data(), nbytes, encode.get(), nbytes);
  EXPECT_EQ(encode_len, compactor_comp_size(buf.data(), nbytes));
  decode_len = compactor_decode(encode.get(), nbytes, decode.get());
  EXPECT_EQ(decode_len, nbytes);
  for (int i = 0; i < nbytes; i++)
    ASSERT_EQ(decode[i], 255) << "i = " << i;
}

TEST(compactor, coding_mixed)
{
  int nbytes = 64;  // 16 ints
  auto buf = std::vector<unsigned char>(nbytes, 0);
  for (int i = 0; i < nbytes / 4; i++)
    buf[i] = 255;
  for (int i = nbytes / 4; i < nbytes / 2; i++)
    buf[i] = i;
  buf[nbytes - 1] = 255;

  // encode `buf`
  auto encode = std::make_unique<unsigned char[]>(nbytes);
  auto encode_len = compactor_encode(buf.data(), nbytes, encode.get(), nbytes);
  EXPECT_EQ(encode_len, compactor_comp_size(buf.data(), nbytes));

  // decode and compare to the original
  auto decode = std::make_unique<unsigned char[]>(nbytes);
  auto decode_len = compactor_decode(encode.get(), nbytes, decode.get());
  EXPECT_EQ(decode_len, nbytes);
  for (int i = 0; i < nbytes; i++)
    ASSERT_EQ(buf[i], decode[i]) << "i = " << i;
}

} // End of the namespace

