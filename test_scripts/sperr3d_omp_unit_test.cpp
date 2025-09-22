#include "SPERR3D_OMP_C.h"
#include "SPERR3D_OMP_D.h"

#include <cstring>
#include "gtest/gtest.h"

namespace {

using sperr::RTNType;

//
// Test constant fields.
//
TEST(sperr3d_constant, one_chunk)
{
  auto input = sperr::read_whole_file<float>("../test_data/const32x20x16.float");
  assert(!input.empty());
  const auto dims = sperr::dims_type{32, 20, 16};
  const auto total_len = dims[0] * dims[1] * dims[2];
  auto inputd = sperr::vecd_type(total_len);
  std::copy(input.cbegin(), input.cend(), inputd.begin());

  // Use an encoder
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, dims);
  encoder.set_psnr(99.0);
  auto rtn = encoder.compress(input.data(), input.size());
  EXPECT_EQ(rtn, RTNType::Good);
  auto stream1 = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  rtn = decoder.use_bitstream(stream1.data(), stream1.size());
  EXPECT_EQ(rtn, RTNType::Good);
  decoder.decompress(stream1.data());
  auto& output = decoder.view_decoded_data();
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(inputd, output);
  EXPECT_EQ(output_dims, dims);

  // Re-use the same objects, and test specifying PWE instead of PSNR.
  encoder.set_tolerance(1.0);
  encoder.compress(input.data(), input.size());
  auto stream2 = encoder.get_encoded_bitstream();

  decoder.use_bitstream(stream2.data(), stream2.size());
  decoder.decompress(stream2.data());
  auto& output2 = decoder.view_decoded_data();
  EXPECT_EQ(inputd, output2);
  EXPECT_EQ(stream2, stream1);
}

TEST(sperr3d_constant, omp_chunks)
{
  auto input = sperr::read_whole_file<float>("../test_data/const32x32x59.float");
  const auto dims = sperr::dims_type{32, 32, 59};
  const auto chunks = sperr::dims_type{32, 16, 32};
  const auto total_len = dims[0] * dims[1] * dims[2];
  auto inputd = sperr::vecd_type(total_len);
  std::copy(input.cbegin(), input.cend(), inputd.begin());

  // Use an encoder
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_psnr(99.0);
  encoder.set_num_threads(3);
  encoder.compress(inputd.data(), inputd.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(4);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  auto& output = decoder.view_decoded_data();
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(inputd, output);
  EXPECT_EQ(output_dims, dims);
}

//
// Test target PWE
//
TEST(sperr3d_target_pwe, small_data_range)
{
  auto input = sperr::read_whole_file<float>("../test_data/vorticity.128_128_41");
  const auto dims = sperr::dims_type{128, 128, 41};
  const auto chunks = sperr::dims_type{64, 64, 41};
  const auto total_len = dims[0] * dims[1] * dims[2];

  // Use an encoder
  double tol = 1.5e-7;
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_tolerance(tol);
  encoder.set_num_threads(4);
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(4);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(output_dims, dims);
  const auto& output = decoder.view_decoded_data();
  for (size_t i = 0; i < input.size(); i++)
    EXPECT_NEAR(input[i], output[i], tol);

  // Test a new tolerance
  tol = 6.7e-6;
  encoder.set_tolerance(tol);
  encoder.compress(input.data(), input.size());
  stream = encoder.get_encoded_bitstream();

  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  const auto& output2 = decoder.view_decoded_data();
  for (size_t i = 0; i < input.size(); i++)
    EXPECT_NEAR(input[i], output2[i], tol);
}

TEST(sperr3d_target_pwe, big)
{
  auto input = sperr::read_whole_file<float>("../test_data/wmag128.float");
  const auto dims = sperr::dims_type{128, 128, 128};
  const auto chunks = sperr::dims_type{64, 70, 80};
  const auto total_len = dims[0] * dims[1] * dims[2];

  // Use an encoder
  double tol = 1.5e-2;
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_tolerance(tol);
  encoder.set_num_threads(4);
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(3);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(output_dims, dims);
  const auto& output = decoder.view_decoded_data();
  for (size_t i = 0; i < input.size(); i++)
    EXPECT_NEAR(input[i], output[i], tol);

  // Test a new tolerance
  tol = 6.7e-1;
  encoder.set_tolerance(tol);
  encoder.compress(input.data(), input.size());
  stream = encoder.get_encoded_bitstream();

  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  const auto& output2 = decoder.view_decoded_data();
  for (size_t i = 0; i < input.size(); i++)
    EXPECT_NEAR(input[i], output2[i], tol);
}

//
// Test target PSNR
//
TEST(sperr3d_target_psnr, big)
{
  auto input = sperr::read_whole_file<float>("../test_data/wmag128.float");
  const auto dims = sperr::dims_type{128, 128, 128};
  const auto chunks = sperr::dims_type{64, 70, 80};
  const auto total_len = dims[0] * dims[1] * dims[2];
  auto inputd = sperr::vecd_type(total_len);
  std::copy(input.begin(), input.end(), inputd.begin());

  // Use an encoder
  double psnr = 88.0;
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_psnr(psnr);
  encoder.set_num_threads(4);
  encoder.compress(inputd.data(), inputd.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(3);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(output_dims, dims);
  const auto& output = decoder.view_decoded_data();
  auto stats = sperr::calc_stats(inputd.data(), output.data(), total_len, 4);
  EXPECT_GT(stats[2], 92.9321);
  EXPECT_LT(stats[2], 92.9322);

  // Test a new psnr
  psnr = 130.0;
  encoder.set_psnr(psnr);
  encoder.compress(input.data(), input.size());
  stream = encoder.get_encoded_bitstream();

  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  const auto& output2 = decoder.view_decoded_data();
  stats = sperr::calc_stats(inputd.data(), output2.data(), total_len, 4);
  EXPECT_GT(stats[2], 134.6098);
  EXPECT_LT(stats[2], 134.6099);
}

TEST(sperr3d_target_psnr, small_data_range)
{
  auto input = sperr::read_whole_file<float>("../test_data/vorticity.128_128_41");
  const auto dims = sperr::dims_type{128, 128, 41};
  const auto chunks = sperr::dims_type{64, 64, 64};
  const auto total_len = dims[0] * dims[1] * dims[2];
  auto inputd = sperr::vecd_type(total_len);
  std::copy(input.begin(), input.end(), inputd.begin());

  // Use an encoder
  double psnr = 88.0;
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_psnr(psnr);
  encoder.set_num_threads(3);
  encoder.compress(inputd.data(), inputd.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(0);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(output_dims, dims);
  const auto& output = decoder.view_decoded_data();
  auto stats = sperr::calc_stats(inputd.data(), output.data(), total_len, 4);
  EXPECT_GT(stats[2], 89.1123);
  EXPECT_LT(stats[2], 89.1124);

  // Test a new psnr
  psnr = 125.0;
  encoder.set_psnr(psnr);
  encoder.compress(input.data(), input.size());
  stream = encoder.get_encoded_bitstream();

  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  const auto& output2 = decoder.view_decoded_data();
  stats = sperr::calc_stats(inputd.data(), output2.data(), total_len, 4);
  EXPECT_GT(stats[2], 126.8866);
  EXPECT_LT(stats[2], 126.8867);
}

//
// Test fixed-size mode
//
TEST(sperr3d_bit_rate, big)
{
  auto input = sperr::read_whole_file<float>("../test_data/wmag128.float");
  const auto dims = sperr::dims_type{128, 128, 128};
  const auto chunks = sperr::dims_type{64, 64, 64};
  const auto total_len = dims[0] * dims[1] * dims[2];
  auto inputd = sperr::vecd_type(total_len);
  std::copy(input.begin(), input.end(), inputd.begin());

  // Use an encoder
  double bpp = 2.0;
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_bitrate(bpp);
  encoder.set_num_threads(4);
  encoder.compress(inputd.data(), inputd.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(0);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(output_dims, dims);
  const auto& output = decoder.view_decoded_data();
  auto stats = sperr::calc_stats(inputd.data(), output.data(), total_len, 4);
  EXPECT_LT(stats[1], 9.6434);
  EXPECT_GT(stats[2], 53.73996);
  EXPECT_LT(stats[2], 53.73997);

  // Test a new bitrate
  bpp = 1.0;
  encoder.set_bitrate(bpp);
  encoder.compress(input.data(), input.size());
  stream = encoder.get_encoded_bitstream();
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data());
  const auto& output2 = decoder.view_decoded_data();
  stats = sperr::calc_stats(inputd.data(), output2.data(), total_len, 4);
  EXPECT_LT(stats[1], 22.3381);
  EXPECT_GT(stats[2], 47.1664);
  EXPECT_LT(stats[2], 47.1665);
}

//
// Test multi-resolution
//
TEST(sperr3d_multi_res, canonical)
{
  auto input = sperr::read_whole_file<float>("../test_data/wmag128.float");
  const auto dims = sperr::dims_type{128, 128, 128};
  const auto chunks = sperr::dims_type{64, 64, 64};
  const auto total_len = dims[0] * dims[1] * dims[2];
  auto inputd = sperr::vecd_type(total_len);
  std::copy(input.begin(), input.end(), inputd.begin());

  // Use an encoder
  double bpp = 2.0;
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks(dims, chunks);
  encoder.set_bitrate(bpp);
  encoder.set_num_threads(4);
  encoder.compress(inputd.data(), inputd.size());
  auto stream = encoder.get_encoded_bitstream();

  // Use a decoder, test the native resolution results.
  auto decoder = sperr::SPERR3D_OMP_D();
  decoder.set_num_threads(0);
  decoder.use_bitstream(stream.data(), stream.size());
  decoder.decompress(stream.data(), true);
  auto output_dims = decoder.get_dims();
  EXPECT_EQ(output_dims, dims);
  const auto& output = decoder.view_decoded_data();
  auto stats = sperr::calc_stats(inputd.data(), output.data(), total_len, 4);
  EXPECT_LT(stats[1], 9.6434);
  EXPECT_GT(stats[2], 53.73996);
  EXPECT_LT(stats[2], 53.73997);

  // Also test the lower-resolution reconstructions.
  auto hierarchy = decoder.release_hierarchy();
  auto resolutions = sperr::coarsened_resolutions(dims, chunks);
  EXPECT_EQ(hierarchy.size(), resolutions.size());
  for (size_t i = 0; i < hierarchy.size(); i++) {
    auto res = resolutions[i];
    EXPECT_EQ(hierarchy[i].size(), res[0] * res[1] * res[2]);
  }
}

}  // anonymous namespace
