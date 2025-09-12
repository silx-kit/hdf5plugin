
#include <random>
#include "gtest/gtest.h"
#include "sperr_helper.h"

namespace {

TEST(sperr_helper, dyadic)
{
  EXPECT_EQ(sperr::can_use_dyadic({64, 1, 1}), std::nullopt);   // 1D
  EXPECT_EQ(sperr::can_use_dyadic({64, 64, 1}), std::nullopt);  // 2D
  EXPECT_EQ(sperr::can_use_dyadic({64, 64, 64}), 3);
  EXPECT_EQ(sperr::can_use_dyadic({128, 128, 128}), 4);
  EXPECT_EQ(sperr::can_use_dyadic({256, 256, 256}), 5);
  EXPECT_EQ(sperr::can_use_dyadic({288, 288, 288}), 6);
  EXPECT_EQ(sperr::can_use_dyadic({256, 256, 300}), 5);
  EXPECT_EQ(sperr::can_use_dyadic({300, 300, 256}), 5);
  EXPECT_EQ(sperr::can_use_dyadic({256, 300, 256}), 5);
}

TEST(sperr_helper, lod_2d)
{
  // Very basic case
  auto dims = sperr::dims_type{64, 64, 1};
  auto lod = sperr::coarsened_resolutions(dims);
  EXPECT_EQ(lod.size(), 3);
  EXPECT_EQ(lod[0], (sperr::dims_type{8, 8, 1}));
  EXPECT_EQ(lod[2], (sperr::dims_type{32, 32, 1}));

  // 2D is simpler, because it's always dyadic!
  dims = {80, 200, 1};
  lod = sperr::coarsened_resolutions(dims);
  EXPECT_EQ(lod.size(), 4);
  EXPECT_EQ(lod[0], (sperr::dims_type{5, 13, 1}));
  EXPECT_EQ(lod[2], (sperr::dims_type{20, 50, 1}));
}

TEST(sperr_helper, lod_3d)
{
  // Very basic case
  auto dims = sperr::dims_type{64, 64, 64};
  auto lod = sperr::coarsened_resolutions(dims);
  EXPECT_EQ(lod.size(), 3);
  EXPECT_EQ(lod[0], (sperr::dims_type{8, 8, 8}));
  EXPECT_EQ(lod[2], (sperr::dims_type{32, 32, 32}));

  // XY has 5 levels, and Z has 6 levels, the overall is 5 levels.
  dims = {144, 144, 288};
  lod = sperr::coarsened_resolutions(dims);
  EXPECT_EQ(lod.size(), 5);
  EXPECT_EQ(lod[0], (sperr::dims_type{5, 5, 9}));
  EXPECT_EQ(lod[2], (sperr::dims_type{18, 18, 36}));
  EXPECT_EQ(lod[4], (sperr::dims_type{72, 72, 144}));

  // Another test
  dims = {300, 300, 160};
  lod = sperr::coarsened_resolutions(dims);
  EXPECT_EQ(lod.size(), 5);
  EXPECT_EQ(lod[0], (sperr::dims_type{10, 10, 5}));
  EXPECT_EQ(lod[2], (sperr::dims_type{38, 38, 20}));
  EXPECT_EQ(lod[4], (sperr::dims_type{150, 150, 80}));

  // Dyadic will not be used, so no coarsened levels.
  dims = {128, 128, 60};
  lod = sperr::coarsened_resolutions(dims);
  EXPECT_EQ(lod.size(), 0);
}

TEST(sperr_helper, lod_3d_multi_chunk)
{
  using sperr::dims_type;

  // If chunk dim is not divisible, there's no available resolution.
  auto vdim = dims_type{90, 90, 90};
  auto cdim = dims_type{60, 60, 60};
  auto res = sperr::coarsened_resolutions(vdim, cdim);
  EXPECT_EQ(res.size(), 0);

  // If the chunk itself doesn't support multi-resolution, then the whole volume doesn't too.
  vdim = {40, 40, 80};
  cdim = {20, 20, 40};
  res = sperr::coarsened_resolutions(vdim, cdim);
  EXPECT_EQ(res.size(), 0);

  // An obvious case.
  vdim = {128, 128, 128};
  cdim = {64, 64, 64};
  res = sperr::coarsened_resolutions(vdim, cdim);
  EXPECT_EQ(res.size(), 3);
  EXPECT_EQ(res[0], (dims_type{16, 16, 16}));
  EXPECT_EQ(res[1], (dims_type{32, 32, 32}));
  EXPECT_EQ(res[2], (dims_type{64, 64, 64}));

  // A case with odd numbers.
  vdim = {156, 147, 177};
  cdim = {39, 49, 59};  // Should result in (4, 3, 3) small chunks.
  res = sperr::coarsened_resolutions(vdim, cdim);
  EXPECT_EQ(res.size(), 3);
  EXPECT_EQ(res[0], (dims_type{20, 21, 24}));
  EXPECT_EQ(res[1], (dims_type{40, 39, 45}));
  EXPECT_EQ(res[2], (dims_type{80, 75, 90}));
}

TEST(sperr_helper, approx_detail_len)
{
  auto len = sperr::calc_approx_detail_len(7, 0);
  EXPECT_EQ(len[0], 7);
  EXPECT_EQ(len[1], 0);
  len = sperr::calc_approx_detail_len(7, 1);
  EXPECT_EQ(len[0], 4);
  EXPECT_EQ(len[1], 3);
  len = sperr::calc_approx_detail_len(8, 1);
  EXPECT_EQ(len[0], 4);
  EXPECT_EQ(len[1], 4);
  len = sperr::calc_approx_detail_len(8, 2);
  EXPECT_EQ(len[0], 2);
  EXPECT_EQ(len[1], 2);
  len = sperr::calc_approx_detail_len(16, 2);
  EXPECT_EQ(len[0], 4);
  EXPECT_EQ(len[1], 4);
}

TEST(sperr_helper, bit_packing)
{
  const size_t num_of_bytes = 11;
  const size_t byte_offset = 1;
  std::vector<bool> input{true,  true,  true,  true,  true,  true,  true,  true,   // 1st byte
                          false, false, false, false, false, false, false, false,  // 2nd byte
                          true,  false, true,  false, true,  false, true,  false,  // 3rd byte
                          false, true,  false, true,  false, true,  false, true,   // 4th byte
                          true,  true,  false, false, true,  true,  false, false,  // 5th byte
                          false, false, true,  true,  false, false, true,  true,   // 6th byte
                          false, false, true,  true,  false, false, true,  false,  // 7th byte
                          true,  false, false, false, true,  true,  true,  false,  // 8th byte
                          false, false, false, true,  false, false, false, true,   // 9th byte
                          true,  true,  true,  false, true,  true,  true,  false,  // 10th byte
                          false, false, true,  true,  true,  false, false, true};  // 11th byte

  auto bytes = sperr::vec8_type(num_of_bytes + byte_offset);

  // Pack booleans
  auto rtn1 = sperr::pack_booleans(bytes, input, byte_offset);
  EXPECT_EQ(rtn1, sperr::RTNType::Good);
  // Unpack booleans
  auto output = std::vector<bool>(num_of_bytes * 8);
  auto rtn2 = sperr::unpack_booleans(output, bytes.data(), bytes.size(), byte_offset);
  EXPECT_EQ(rtn2, sperr::RTNType::Good);

  if (rtn1 == sperr::RTNType::Good && rtn1 == rtn2) {
    for (size_t i = 0; i < input.size(); i++)
      EXPECT_EQ(input[i], output[i]);
  }
}

TEST(sperr_helper, bit_packing_one_byte)
{
  // All true
  auto input = std::array<bool, 8>{true, true, true, true, true, true, true, true};
  // Pack booleans
  auto byte = sperr::pack_8_booleans(input);
  // Unpack booleans
  auto output = sperr::unpack_8_booleans(byte);
  for (size_t i = 0; i < 8; i++)
    EXPECT_EQ(input[i], output[i]);

  // Odd locations false
  for (size_t i = 1; i < 8; i += 2)
    input[i] = false;
  byte = sperr::pack_8_booleans(input);
  output = sperr::unpack_8_booleans(byte);
  for (size_t i = 0; i < 8; i++)
    EXPECT_EQ(input[i], output[i]);

  // All false
  for (bool& val : input)
    val = false;
  byte = sperr::pack_8_booleans(input);
  output = sperr::unpack_8_booleans(byte);
  for (size_t i = 0; i < 8; i++)
    EXPECT_EQ(input[i], output[i]);

  // Odd locations true
  for (size_t i = 1; i < 8; i += 2)
    input[i] = true;
  byte = sperr::pack_8_booleans(input);
  output = sperr::unpack_8_booleans(byte);
  for (size_t i = 0; i < 8; i++)
    EXPECT_EQ(input[i], output[i]);
}

TEST(sperr_helper, bit_packing_1032_bools)
{
  const size_t num_of_bits = 1032;  // 1024 + 8, so the last stride is partial.
  const size_t num_of_bytes = num_of_bits / 8;
  const size_t byte_offset = 23;

  std::random_device rd;   // Will be used to obtain a seed for the random number engine
  std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with rd()
  std::uniform_int_distribution<> distrib(0, 2);
  auto input = std::vector<bool>(num_of_bits);
  auto bytes = sperr::vec8_type(num_of_bytes + byte_offset);
  for (size_t i = 0; i < num_of_bits; i++)
    input[i] = distrib(gen);

  // Pack booleans
  auto rtn1 = sperr::pack_booleans(bytes, input, byte_offset);
  EXPECT_EQ(rtn1, sperr::RTNType::Good);
  // Unpack booleans
  auto output = std::vector<bool>(num_of_bits);
  auto rtn2 = sperr::unpack_booleans(output, bytes.data(), bytes.size(), byte_offset);
  EXPECT_EQ(rtn2, sperr::RTNType::Good);

  if (rtn1 == sperr::RTNType::Good && rtn1 == rtn2) {
    for (size_t i = 0; i < input.size(); i++)
      EXPECT_EQ(input[i], output[i]);
  }
}

TEST(sperr_helper, domain_decomposition)
{
  using sperr::dims_type;
  using cdef = std::array<size_t, 6>;  // chunk definition

  auto vol = dims_type{4, 4, 4};
  auto subd = dims_type{1, 2, 3};

  auto chunks = sperr::chunk_volume(vol, subd);
  EXPECT_EQ(chunks.size(), 8);
  EXPECT_EQ(chunks[0], (cdef{0, 1, 0, 2, 0, 4}));
  EXPECT_EQ(chunks[1], (cdef{1, 1, 0, 2, 0, 4}));
  EXPECT_EQ(chunks[2], (cdef{2, 1, 0, 2, 0, 4}));
  EXPECT_EQ(chunks[3], (cdef{3, 1, 0, 2, 0, 4}));

  EXPECT_EQ(chunks[4], (cdef{0, 1, 2, 2, 0, 4}));
  EXPECT_EQ(chunks[5], (cdef{1, 1, 2, 2, 0, 4}));
  EXPECT_EQ(chunks[6], (cdef{2, 1, 2, 2, 0, 4}));
  EXPECT_EQ(chunks[7], (cdef{3, 1, 2, 2, 0, 4}));

  vol = dims_type{4, 4, 1};  // will essentially require a decomposition on a 2D plane.
  chunks = sperr::chunk_volume(vol, subd);
  EXPECT_EQ(chunks.size(), 8);
  EXPECT_EQ(chunks[0], (cdef{0, 1, 0, 2, 0, 1}));
  EXPECT_EQ(chunks[1], (cdef{1, 1, 0, 2, 0, 1}));
  EXPECT_EQ(chunks[2], (cdef{2, 1, 0, 2, 0, 1}));
  EXPECT_EQ(chunks[3], (cdef{3, 1, 0, 2, 0, 1}));

  EXPECT_EQ(chunks[4], (cdef{0, 1, 2, 2, 0, 1}));
  EXPECT_EQ(chunks[5], (cdef{1, 1, 2, 2, 0, 1}));
  EXPECT_EQ(chunks[6], (cdef{2, 1, 2, 2, 0, 1}));
  EXPECT_EQ(chunks[7], (cdef{3, 1, 2, 2, 0, 1}));
}

TEST(sperr_helper, read_sections)
{
  // Create an array, and write to disk.
  auto vec = std::vector<uint8_t>(256, 0);
  std::iota(vec.begin(), vec.end(), 0);
  sperr::write_n_bytes("test.tmp", 256, vec.data());
  auto buf = sperr::vec8_type();

  // Create a section that exceeds the file size.
  auto secs = std::vector<size_t>(2, 0);
  secs.insert(secs.end(), {200, 56});
  secs.insert(secs.end(), {101, 156});
  auto rtn = sperr::read_sections("test.tmp", secs, buf);
  EXPECT_EQ(rtn, sperr::RTNType::WrongLength);

  // Pop out the offending section requests.
  secs.pop_back();
  secs.pop_back();
  rtn = sperr::read_sections("test.tmp", secs, buf);
  EXPECT_EQ(rtn, sperr::RTNType::Good);

  // Add another section, and try reading them.
  secs.insert(secs.end(), {30, 5});
  buf.assign(10, 1);
  sperr::read_sections("test.tmp", secs, buf);
  EXPECT_EQ(buf.size(), 71);
  for (size_t i = 0; i < 10; i++)  // First 10 elements should remain the same.
    EXPECT_EQ(buf[i], 1);
  for (size_t i = 0; i < 56; i++)  // Next 56 elements should start from 200.
    EXPECT_EQ(buf[i + 10], i + 200);
  for (size_t i = 0; i < 5; i++)  // Next 5 elements should start from 30.
    EXPECT_EQ(buf[i + 66], i + 30);

  // Test the extract version too. Read and extract should give the same results.
  buf.clear();
  sperr::read_sections("test.tmp", secs, buf);
  auto full_input = sperr::read_whole_file<uint8_t>("test.tmp");
  auto buf2 = sperr::vec8_type();
  auto rtn2 = sperr::extract_sections(full_input.data(), full_input.size(), secs, buf2);
  EXPECT_EQ(rtn, sperr::RTNType::Good);
  EXPECT_EQ(buf, buf2);
}

}  // namespace
