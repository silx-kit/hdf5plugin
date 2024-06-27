#include "SPECK1D_INT_DEC.h"
#include "SPECK1D_INT_ENC.h"

#include "SPECK2D_INT_DEC.h"
#include "SPECK2D_INT_ENC.h"

#include "SPECK3D_INT_DEC.h"
#include "SPECK3D_INT_ENC.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace {

template <typename T>
auto ProduceRandomArray(size_t len, float stddev, uint32_t seed)
    -> std::pair<std::vector<T>, sperr::Bitmask>
{
  std::mt19937 gen{seed};
  std::normal_distribution<float> d{0.0, stddev};

  auto tmp = std::vector<float>(len);
  std::generate(tmp.begin(), tmp.end(), [&gen, &d]() { return d(gen); });

  auto coeffs = std::vector<T>(len);
  auto signs = sperr::Bitmask(len);
  signs.reset_true();
  for (size_t i = 0; i < len; i++) {
    auto l = std::lround(std::abs(tmp[i]));
    if (l > std::numeric_limits<T>::max())
      coeffs[i] = std::numeric_limits<T>::max();
    else
      coeffs[i] = l;
    // Only specify signs for non-zero values.
    if (coeffs[i] != 0)
      signs.wbit(i, (tmp[i] > 0.f));
  }

  return {coeffs, signs};
}

//
// Start 1D test cases
//
TEST(SPECK1D_INT, minimal)
{
  const auto dims = sperr::dims_type{40, 1, 1};

  auto input = std::vector<uint8_t>(dims[0], 0);
  auto input_signs = sperr::Bitmask(input.size());
  input_signs.reset_true();
  input[4] = 1;
  input[7] = 3;
  input_signs.wfalse(7);
  input[10] = 7;
  input[11] = 9;
  input_signs.wfalse(11);
  input[16] = 10;
  input[19] = 12;
  input_signs.wfalse(19);
  input[26] = 18;
  input[29] = 19;
  input_signs.wfalse(29);
  input[32] = 32;
  input[39] = 32;
  input_signs.wfalse(39);

  //
  // Test 1-byte integer
  //
  {
    auto encoder = sperr::SPECK1D_INT_ENC<uint8_t>();
    encoder.use_coeffs(input, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK1D_INT_DEC<uint8_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 1);
    EXPECT_EQ(decoder.integer_len(), 1);
    EXPECT_EQ(input, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 2-byte integer
  //
  auto input16 = std::vector<uint16_t>(dims[0], 0);
  std::copy(input.cbegin(), input.cend(), input16.begin());
  input16[30] = 300;
  {
    auto encoder = sperr::SPECK1D_INT_ENC<uint16_t>();
    encoder.use_coeffs(input16, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK1D_INT_DEC<uint16_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 2);
    EXPECT_EQ(decoder.integer_len(), 2);
    EXPECT_EQ(input16, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 4-byte integer
  //
  auto input32 = std::vector<uint32_t>(dims[0], 0);
  std::copy(input16.cbegin(), input16.cend(), input32.begin());
  input32[20] = 70'000;
  {
    auto encoder = sperr::SPECK1D_INT_ENC<uint32_t>();
    encoder.use_coeffs(input32, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK1D_INT_DEC<uint32_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 4);
    EXPECT_EQ(decoder.integer_len(), 4);
    EXPECT_EQ(input32, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 8-byte integer
  //
  auto input64 = std::vector<uint64_t>(dims[0], 0);
  std::copy(input32.cbegin(), input32.cend(), input64.begin());
  input64[23] = 5'000'700'000;
  {
    auto encoder = sperr::SPECK1D_INT_ENC<uint64_t>();
    encoder.use_coeffs(input64, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK1D_INT_DEC<uint64_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 8);
    EXPECT_EQ(decoder.integer_len(), 8);
    EXPECT_EQ(input64, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }
}

TEST(SPECK1D_INT, Random1)
{
  const auto dims = sperr::dims_type{2000, 1, 1};

  auto [input, input_signs] = ProduceRandomArray<uint8_t>(dims[0], 2.9, 1);

  // Encode
  auto encoder = sperr::SPECK1D_INT_ENC<uint8_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  auto bitstream = sperr::vec8_type();
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK1D_INT_DEC<uint8_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

TEST(SPECK1D_INT, Random2)
{
  const auto dims = sperr::dims_type{63 * 79 * 128, 1, 1};

  auto [input, input_signs] = ProduceRandomArray<uint16_t>(dims[0], 499.0, 2);

  // Encode
  auto encoder = sperr::SPECK1D_INT_ENC<uint16_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  sperr::vec8_type bitstream;
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK1D_INT_DEC<uint16_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

TEST(SPECK1D_INT, RandomRandom)
{
  const auto dims = sperr::dims_type{63 * 64 * 119, 1, 1};

  auto rd = std::random_device();
  auto [input, input_signs] = ProduceRandomArray<uint64_t>(dims[0], 8345.3, rd());

  // Encode
  auto encoder = sperr::SPECK1D_INT_ENC<uint64_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  auto bitstream = sperr::vec8_type();
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK1D_INT_DEC<uint64_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

//
// Starting 2D test cases
//
TEST(SPECK2D_INT, minimal)
{
  const auto dims = sperr::dims_type{9, 8, 1};

  auto input = std::vector<uint8_t>(dims[0] * dims[1], 0);
  auto input_signs = sperr::Bitmask(input.size());
  input_signs.reset_true();
  input[2] = 1;
  input_signs.wfalse(2);
  input[4] = 1;
  input[7] = 3;
  input_signs.wfalse(7);
  input[10] = 7;
  input[11] = 9;
  input_signs.wfalse(11);
  input[16] = 10;
  input[19] = 12;
  input_signs.wfalse(19);
  input[26] = 18;
  input[29] = 19;
  input_signs.wfalse(29);
  input[32] = 32;
  input[39] = 32;
  input_signs.wfalse(39);
  input[70] = 8;
  input_signs.wfalse(70);
  input[71] = 255;

  //
  // Test 1-byte integer
  //
  {
    auto encoder = sperr::SPECK2D_INT_ENC<uint8_t>();
    encoder.use_coeffs(input, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK2D_INT_DEC<uint8_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 1);
    EXPECT_EQ(decoder.integer_len(), 1);
    EXPECT_EQ(input, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 2-byte integer
  //
  auto input16 = std::vector<uint16_t>(input.size());
  std::copy(input.cbegin(), input.cend(), input16.begin());
  input16[71] = 300;
  {
    auto encoder = sperr::SPECK2D_INT_ENC<uint16_t>();
    encoder.use_coeffs(input16, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK2D_INT_DEC<uint16_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 2);
    EXPECT_EQ(decoder.integer_len(), 2);
    EXPECT_EQ(input16, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 4-byte integer
  //
  auto input32 = std::vector<uint32_t>(input.size());
  std::copy(input16.cbegin(), input16.cend(), input32.begin());
  input32[66] = 70'000;
  {
    auto encoder = sperr::SPECK2D_INT_ENC<uint32_t>();
    encoder.use_coeffs(input32, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK2D_INT_DEC<uint32_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 4);
    EXPECT_EQ(decoder.integer_len(), 4);
    EXPECT_EQ(input32, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 8-byte integer
  //
  auto input64 = std::vector<uint64_t>(input.size());
  std::copy(input32.cbegin(), input32.cend(), input64.begin());
  input64[57] = 5'000'700'000;
  {
    auto encoder = sperr::SPECK2D_INT_ENC<uint64_t>();
    encoder.use_coeffs(input64, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK2D_INT_DEC<uint64_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 8);
    EXPECT_EQ(decoder.integer_len(), 8);
    EXPECT_EQ(input64, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }
}

TEST(SPECK2D_INT, Random1)
{
  const auto dims = sperr::dims_type{9, 20, 1};

  auto [input, input_signs] = ProduceRandomArray<uint8_t>(dims[0] * dims[1], 2.9, 1);

  // Encode
  auto encoder = sperr::SPECK2D_INT_ENC<uint8_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  auto bitstream = sperr::vec8_type();
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK2D_INT_DEC<uint8_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

TEST(SPECK2D_INT, Random2)
{
  const auto dims = sperr::dims_type{63, 130, 1};

  auto [input, input_signs] = ProduceRandomArray<uint16_t>(dims[0] * dims[1], 499.0, 2);

  // Encode
  auto encoder = sperr::SPECK2D_INT_ENC<uint16_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  sperr::vec8_type bitstream;
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK2D_INT_DEC<uint16_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

TEST(SPECK2D_INT, RandomRandom)
{
  const auto dims = sperr::dims_type{63, 256, 1};

  auto rd = std::random_device();
  auto [input, input_signs] = ProduceRandomArray<uint64_t>(dims[0] * dims[1], 8345.3, rd());

  // Encode
  auto encoder = sperr::SPECK2D_INT_ENC<uint64_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  auto bitstream = sperr::vec8_type();
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK2D_INT_DEC<uint64_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

//
// Starting 3D test cases
//
TEST(SPECK3D_INT, minimal)
{
  const auto dims = sperr::dims_type{4, 3, 8};
  const auto total_vals = dims[0] * dims[1] * dims[2];
  auto input = std::vector<uint8_t>(total_vals, 0);
  auto input_signs = sperr::Bitmask(input.size());
  input_signs.reset_true();
  input[4] = 1;
  input[7] = 3;
  input_signs.wfalse(7);
  input[10] = 7;
  input[11] = 9;
  input_signs.wfalse(11);
  input[16] = 10;
  input[19] = 12;
  input_signs.wfalse(19);
  input[26] = 18;
  input[29] = 19;
  input_signs.wfalse(29);
  input[32] = 32;
  input[49] = 32;
  input_signs.wfalse(49);
  // Construct a value that's in between of signed and unsigned 8-bit int.
  input[42] = uint8_t(std::numeric_limits<int8_t>::max()) + 8;

  //
  // Test 1-byte integers
  //
  {
    auto encoder = sperr::SPECK3D_INT_ENC<uint8_t>();
    encoder.use_coeffs(input, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK3D_INT_DEC<uint8_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 1);
    EXPECT_EQ(decoder.integer_len(), 1);
    EXPECT_EQ(input, output);
    EXPECT_EQ(input, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 2-byte integers
  //
  auto input16 = std::vector<uint16_t>(total_vals, 0);
  std::copy(input.begin(), input.end(), input16.begin());
  input16[30] = 300;
  input_signs.wfalse(30);
  // Construct a value that's in between of signed and unsigned 16-bit int.
  input16[39] = uint16_t(std::numeric_limits<int16_t>::max()) + 16;
  input_signs.wfalse(39);
  {
    auto encoder = sperr::SPECK3D_INT_ENC<uint16_t>();
    encoder.use_coeffs(input16, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK3D_INT_DEC<uint16_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 2);
    EXPECT_EQ(decoder.integer_len(), 2);
    EXPECT_EQ(input16, output);
    EXPECT_EQ(input16, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 4-byte integers
  //
  auto input32 = std::vector<uint32_t>(total_vals, 0);
  std::copy(input16.begin(), input16.end(), input32.begin());
  input32[20] = 7'0300;
  input_signs.wfalse(20);
  // Construct a value that's in between of signed and unsigned 32-bit int.
  input32[55] = uint32_t(std::numeric_limits<int32_t>::max()) + 32;
  {
    auto encoder = sperr::SPECK3D_INT_ENC<uint32_t>();
    encoder.use_coeffs(input32, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK3D_INT_DEC<uint32_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 4);
    EXPECT_EQ(decoder.integer_len(), 4);
    EXPECT_EQ(input32, output);
    EXPECT_EQ(input32, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }

  //
  // Test 8-byte integers
  //
  auto input64 = std::vector<uint64_t>(total_vals, 0);
  std::copy(input32.begin(), input32.end(), input64.begin());
  input64[27] = 5'000'700'990;
  input_signs.wfalse(27);
  // Construct a value that's in between of signed and unsigned 64-bit int.
  input64[68] = uint64_t(std::numeric_limits<int64_t>::max()) + 64;
  input_signs.wfalse(68);
  {
    auto encoder = sperr::SPECK3D_INT_ENC<uint64_t>();
    encoder.use_coeffs(input64, input_signs);
    encoder.set_dims(dims);
    encoder.encode();
    auto bitstream = sperr::vec8_type();
    encoder.append_encoded_bitstream(bitstream);

    auto decoder = sperr::SPECK3D_INT_DEC<uint64_t>();
    decoder.set_dims(dims);
    decoder.use_bitstream(bitstream.data(), bitstream.size());
    decoder.decode();
    auto output = decoder.release_coeffs();
    auto output_signs = decoder.release_signs();

    EXPECT_EQ(encoder.integer_len(), 8);
    EXPECT_EQ(decoder.integer_len(), 8);
    EXPECT_EQ(input64, output);
    EXPECT_EQ(input64, output);
    EXPECT_EQ(input_signs.size(), output_signs.size());
    for (size_t i = 0; i < input_signs.size(); i++)
      EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
  }
}

TEST(SPECK3D_INT, Random1)
{
  const auto dims = sperr::dims_type{10, 20, 30};
  const auto total_vals = dims[0] * dims[1] * dims[2];

  auto [input, input_signs] = ProduceRandomArray<uint8_t>(total_vals, 2.9, 1);

  // Encode
  auto encoder = sperr::SPECK3D_INT_ENC<uint8_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  sperr::vec8_type bitstream;
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK3D_INT_DEC<uint8_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

TEST(SPECK3D_INT, Random2)
{
  const auto dims = sperr::dims_type{63, 79, 128};
  const auto total_vals = dims[0] * dims[1] * dims[2];

  auto [input, input_signs] = ProduceRandomArray<uint16_t>(total_vals, 499.0, 2);

  // Encode
  auto encoder = sperr::SPECK3D_INT_ENC<uint16_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  sperr::vec8_type bitstream;
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK3D_INT_DEC<uint16_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

TEST(SPECK3D_INT, RandomRandom)
{
  const auto dims = sperr::dims_type{63, 64, 119};
  const auto total_vals = dims[0] * dims[1] * dims[2];

  auto rd = std::random_device();
  auto [input, input_signs] = ProduceRandomArray<uint64_t>(total_vals, 8345.3, rd());

  // Encode
  auto encoder = sperr::SPECK3D_INT_ENC<uint64_t>();
  encoder.use_coeffs(input, input_signs);
  encoder.set_dims(dims);
  encoder.encode();
  sperr::vec8_type bitstream;
  encoder.append_encoded_bitstream(bitstream);

  // Decode
  auto decoder = sperr::SPECK3D_INT_DEC<uint64_t>();
  decoder.set_dims(dims);
  decoder.use_bitstream(bitstream.data(), bitstream.size());
  decoder.decode();
  auto output = decoder.release_coeffs();
  auto output_signs = decoder.release_signs();

  EXPECT_EQ(input, output);
  EXPECT_EQ(input_signs.size(), output_signs.size());
  for (size_t i = 0; i < input_signs.size(); i++)
    EXPECT_EQ(input_signs.rbit(i), output_signs.rbit(i));
}

}  // namespace
