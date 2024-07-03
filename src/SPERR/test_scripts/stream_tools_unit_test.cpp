#include "SPERR3D_OMP_C.h"
#include "SPERR3D_Stream_Tools.h"

#include "gtest/gtest.h"

namespace {

using sperr::RTNType;

// Test constant field
TEST(stream_tools, constant_1chunk)
{
  // Produce a bitstream to disk
  auto input = sperr::read_whole_file<float>("../test_data/const32x20x16.float");
  assert(!input.empty());
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks({32, 20, 16}, {32, 20, 16});
  encoder.set_psnr(99.0);
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();
  auto filename = std::string("./test.tmp");
  sperr::write_n_bytes(filename, stream.size(), stream.data());

  // Test progressive read!
  auto tools = sperr::SPERR3D_Stream_Tools();
  auto part = tools.progressive_read(filename, 100);

  // The returned bitstream should remain the same.
  EXPECT_EQ(part, stream);

  // If truncate from memory, the result should remain the same.
  auto trunc = tools.progressive_truncate(stream.data(), stream.size(), 100);
  EXPECT_EQ(trunc, stream);
}

TEST(stream_tools, constant_nchunks)
{
  // Produce a bitstream to disk
  auto input = sperr::read_whole_file<float>("../test_data/const32x20x16.float");
  assert(!input.empty());
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks({32, 20, 16}, {10, 10, 8});
  encoder.set_psnr(99.0);
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();
  auto filename = std::string("./test.tmp");
  sperr::write_n_bytes(filename, stream.size(), stream.data());

  // Test progressive read!
  auto tools = sperr::SPERR3D_Stream_Tools();
  auto part = tools.progressive_read(filename, 50);

  // The returned bitstream should still remain the same, except than one bit, because
  // each chunk is so small that it's still kept in whole.
  EXPECT_EQ(part.size(), stream.size());
  EXPECT_EQ(part[0], stream[0]);
  EXPECT_EQ(part[1], stream[1] + 128);
  for (size_t i = 2; i < part.size(); i++)
    EXPECT_EQ(part[i], stream[i]);

  // If truncate from memory, the result should remain the same.
  auto trunc = tools.progressive_truncate(stream.data(), stream.size(), 50);
  EXPECT_EQ(trunc, part);
}

//
// Test a non-constant field.
//
TEST(stream_tools, regular_1chunk)
{
  // Produce a bitstream to disk
  auto filename = std::string("./test.tmp");
  auto input = sperr::read_whole_file<float>("../test_data/vorticity.128_128_41");
  assert(!input.empty());
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks({128, 128, 41}, {128, 128, 41});
  encoder.set_psnr(100.0);  // Resulting about 9.2bpp.
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();
  sperr::write_n_bytes(filename, stream.size(), stream.data());

  // Test progressive read!
  auto tools = sperr::SPERR3D_Stream_Tools();
  auto header = tools.get_stream_header(stream.data());
  auto part = tools.progressive_read(filename, 50);

  // Header should (mostly) remain the same.
  EXPECT_EQ(part[0], stream[0]);
  EXPECT_EQ(part[1], stream[1] + 128);
  for (size_t i = 2; i < header.header_len - 4; i++)  // Exclude the last 4 bytes (chunk len).
    EXPECT_EQ(part[i], stream[i]);

  // The remaining bytes of each chunk should also remain the same.
  auto header2 = tools.get_stream_header(part.data());
  EXPECT_EQ(header.chunk_offsets.size(), header2.chunk_offsets.size());

  for (size_t i = 0; i < header.chunk_offsets.size() / 2; i++) {
    auto orig_start = header.chunk_offsets[i * 2];
    auto part_start = header2.chunk_offsets[i * 2];
    for (size_t j = 0; j < header2.chunk_offsets[i * 2 + 1]; j++)
      EXPECT_EQ(stream[orig_start + j], part[part_start + j]);
  }

  // If truncate from memory, the result should remain the same.
  auto trunc = tools.progressive_truncate(stream.data(), stream.size(), 50);
  EXPECT_EQ(trunc, part);
}

TEST(stream_tools, regular_nchunks)
{
  // Produce a bitstream to disk
  auto filename = std::string("./test.tmp");
  auto input = sperr::read_whole_file<float>("../test_data/vorticity.128_128_41");
  assert(!input.empty());
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks({128, 128, 41}, {31, 40, 21});
  encoder.set_psnr(100.0);  // Resulting about 10bpp.
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();
  sperr::write_n_bytes(filename, stream.size(), stream.data());

  // Test progressive read!
  auto tools = sperr::SPERR3D_Stream_Tools();
  auto header = tools.get_stream_header(stream.data());
  auto part = tools.progressive_read(filename, 35);

  // Header should (mostly) remain the same.
  EXPECT_EQ(part[0], stream[0]);
  EXPECT_EQ(part[1], stream[1] + 128);

  // The remaining bytes should also remain the same.
  auto header2 = tools.get_stream_header(part.data());
  EXPECT_EQ(header.chunk_offsets.size(), header2.chunk_offsets.size());

  for (size_t i = 0; i < header.chunk_offsets.size() / 2; i++) {
    auto orig_start = header.chunk_offsets[i * 2];
    auto part_start = header2.chunk_offsets[i * 2];
    for (size_t j = 0; j < header2.chunk_offsets[i * 2 + 1]; j++)
      EXPECT_EQ(stream[orig_start + j], part[part_start + j]);
  }

  // If truncate from memory, the result should remain the same.
  auto trunc = tools.progressive_truncate(stream.data(), stream.size(), 35);
  EXPECT_EQ(trunc, part);
}

TEST(stream_tools, min_chunk_len)
{
  // Produce a bitstream to disk
  auto filename = std::string("./test.tmp");
  auto input = sperr::read_whole_file<float>("../test_data/wmag17.float");
  assert(!input.empty());
  auto encoder = sperr::SPERR3D_OMP_C();
  encoder.set_dims_and_chunks({17, 17, 17}, {8, 8, 8});
  encoder.set_psnr(100.0);
  encoder.compress(input.data(), input.size());
  auto stream = encoder.get_encoded_bitstream();
  sperr::write_n_bytes(filename, stream.size(), stream.data());

  // Test progressive read! The requested 1% of bytes results in every chunk to have ~10 bytes,
  // which would be changed to 64 bytes eventually, and not resulting in failed assertions.
  auto tools = sperr::SPERR3D_Stream_Tools();
  auto header = tools.get_stream_header(stream.data());
  auto part = tools.progressive_read(filename, 1);

  // Header should (mostly) remain the same.
  EXPECT_EQ(part[0], stream[0]);
  EXPECT_EQ(part[1], stream[1] + 128);

  // The header of each chunk (first 64 bytes) should also remain the same.
  auto header2 = tools.get_stream_header(part.data());
  EXPECT_EQ(header.chunk_offsets.size(), header2.chunk_offsets.size());

  for (size_t i = 0; i < header.chunk_offsets.size() / 2; i++) {
    auto orig_start = header.chunk_offsets[i * 2];
    auto part_start = header2.chunk_offsets[i * 2];
    for (size_t j = 0; j < header2.chunk_offsets[i * 2 + 1]; j++)
      EXPECT_EQ(stream[orig_start + j], part[part_start + j]);
  }

  // If truncate from memory, the result should remain the same.
  auto trunc = tools.progressive_truncate(stream.data(), stream.size(), 1);
  EXPECT_EQ(trunc, part);
}

}  // anonymous namespace
