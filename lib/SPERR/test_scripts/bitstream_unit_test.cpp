#include "gtest/gtest.h"

#include "Bitmask.h"
#include "Bitstream.h"

#include <random>
#include <vector>

namespace {

using Stream = sperr::Bitstream;
using Mask = sperr::Bitmask;

TEST(Bitstream, constructor)
{
  auto s1 = Stream();
  EXPECT_EQ(s1.capacity(), 0);

  auto s2 = Stream(1024 + 1);
  EXPECT_EQ(s2.capacity(), 1024 + 64);

  auto s3 = Stream(1024 + 63);
  EXPECT_EQ(s3.capacity(), 1024 + 64);

  auto s4 = Stream(1024 + 64);
  EXPECT_EQ(s4.capacity(), 1024 + 64);

  auto s5 = Stream(1024 + 65);
  EXPECT_EQ(s5.capacity(), 1024 + 64 * 2);

  auto s6 = Stream(1024 - 1);
  EXPECT_EQ(s6.capacity(), 1024);

  auto s7 = Stream(1024 - 63);
  EXPECT_EQ(s7.capacity(), 1024);

  auto s8 = Stream(1024 - 64);
  EXPECT_EQ(s8.capacity(), 1024 - 64);

  auto s9 = Stream(1024 - 65);
  EXPECT_EQ(s9.capacity(), 1024 - 64);
}

TEST(Bitstream, MemoryAllocation1)
{
  auto s1 = Stream(64);
  auto vec = std::vector<bool>();

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib(0, 1);

  s1.rewind();
  for (size_t i = 0; i < 64; i++) {
    auto val = distrib(gen);
    s1.wbit(val);
    vec.push_back(val);
  }
  EXPECT_EQ(s1.capacity(), 64);

  // Write another bit and flush; the capacity is expected to grow to 128.
  s1.wbit(1);
  vec.push_back(1);
  EXPECT_EQ(s1.wtell(), 65);
  EXPECT_EQ(s1.capacity(), 64);
  s1.flush();
  EXPECT_EQ(s1.capacity(), 128);

  // All saved bits should be correct too.
  s1.rewind();
  for (size_t i = 0; i < vec.size(); i++)
    EXPECT_EQ(s1.rbit(), vec[i]) << "at idx = " << i;

  // Let's try to trigger another memory re-allocation
  s1.wseek(65);
  for (size_t i = 0; i < 64; i++) {
    auto val = distrib(gen);
    s1.wbit(val);
    vec.push_back(val);
  }
  EXPECT_EQ(s1.wtell(), 129);
  EXPECT_EQ(s1.capacity(), 128);
  s1.flush();
  EXPECT_EQ(s1.capacity(), 192);
  s1.rewind();
  for (size_t i = 0; i < vec.size(); i++)
    EXPECT_EQ(s1.rbit(), vec[i]) << "at idx = " << i;
}

TEST(Bitstream, StreamWriteRead)
{
  const size_t N = 150;
  auto s1 = Stream();
  auto vec = std::vector<bool>(N);

  // Make N writes
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib1(0, 1);
  for (size_t i = 0; i < N; i++) {
    const bool bit = distrib1(gen);
    vec[i] = bit;
    s1.wbit(bit);
  }
  EXPECT_EQ(s1.wtell(), 150);
  s1.flush();
  EXPECT_EQ(s1.wtell(), 192);

  s1.rewind();
  for (size_t i = 0; i < N; i++)
    EXPECT_EQ(s1.rbit(), vec[i]) << " at idx = " << i;
}

TEST(Bitstream, RandomWriteRead)
{
  const size_t N = 256;
  auto s1 = Stream(59);
  auto vec = std::vector<bool>(N);

  // Make N stream writes
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib1(0, 1);
  for (size_t i = 0; i < N; i++) {
    const bool bit = distrib1(gen);
    vec[i] = bit;
    s1.wbit(bit);
  }
  EXPECT_EQ(s1.wtell(), 256);
  s1.flush();
  EXPECT_EQ(s1.wtell(), 256);

  // Make random writes on word boundaries
  s1.wseek(63);
  s1.wbit(true);
  vec[63] = true;
  s1.wseek(127);
  s1.wbit(false);
  vec[127] = false;
  s1.wseek(191);
  s1.wbit(true);
  vec[191] = true;
  s1.wseek(255);
  s1.wbit(false);
  vec[255] = false;
  s1.rewind();
  for (size_t i = 0; i < N; i++)
    EXPECT_EQ(s1.rbit(), vec[i]) << " at idx = " << i;

  // Make random reads
  std::uniform_int_distribution<unsigned int> distrib2(0, N - 1);
  for (size_t i = 0; i < 100; i++) {
    const auto pos = distrib2(gen);
    s1.rseek(pos);
    EXPECT_EQ(s1.rbit(), vec[pos]) << " at idx = " << i;
  }
}

TEST(Bitstream, CompactStream)
{
  // Test full 64-bit multiples
  size_t N = 128;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib1(0, 1);
  auto s1 = Stream();

  for (size_t i = 0; i < N; i++)
    s1.wbit(distrib1(gen));
  s1.flush();

  auto buf = s1.get_bitstream(N);
  EXPECT_EQ(buf.size(), 16);
  s1.rewind();
  auto s2 = Stream();
  s2.parse_bitstream(buf.data(), 128);
  for (size_t i = 0; i < N; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());

  // Test full 64-bit multiples and 8-bit multiples
  buf = s1.get_bitstream(80);
  EXPECT_EQ(buf.size(), 10);
  s1.rewind();
  s2.parse_bitstream(buf.data(), 80);
  for (size_t i = 0; i < 80; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());

  // Test full 64-bit multiples, 8-bit multiples, and remaining bits
  buf = s1.get_bitstream(85);
  EXPECT_EQ(buf.size(), 11);
  s1.rewind();
  s2.parse_bitstream(buf.data(), 85);
  for (size_t i = 0; i < 85; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());

  // Test less than 64 bits
  buf = s1.get_bitstream(45);
  EXPECT_EQ(buf.size(), 6);
  s1.rewind();
  s2.parse_bitstream(buf.data(), 45);
  for (size_t i = 0; i < 45; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());

  // Test less than 8 bits
  buf = s1.get_bitstream(5);
  EXPECT_EQ(buf.size(), 1);
  s1.rewind();
  s2.parse_bitstream(buf.data(), 5);
  for (size_t i = 0; i < 5; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());
}

TEST(Bitstream, Reserve)
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib1(0, 1);

  // Reserve on an empty stream.
  auto s1 = Stream();
  s1.reserve(30);
  for (size_t i = 0; i < s1.capacity(); i++)
    EXPECT_EQ(s1.rbit(), false);

  // Reserve on a stream that's been written.
  auto s2 = Stream();
  s1.rewind();
  for (size_t i = 0; i < 30; i++) {
    auto bit = distrib1(gen);
    s1.wbit(bit);
    s2.wbit(bit);
  }
  s1.flush();
  s2.flush();
  s1.reserve(100);
  s1.rewind();
  s2.rewind();
  for (size_t i = 0; i < 30; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());
  for (size_t i = 30; i < s1.capacity(); i++)
    EXPECT_EQ(s1.rbit(), false);

  // Reserve on a stream that's from parsing another bitstream.
  s1.wseek(30);
  for (size_t i = 0; i < 41; i++)
    s1.wbit(distrib1(gen));
  s1.flush();
  auto buf = s1.get_bitstream(71);
  s2.parse_bitstream(buf.data(), 71);
  s1.rewind();
  s2.rewind();
  for (size_t i = 0; i < 71; i++)
    EXPECT_EQ(s1.rbit(), s2.rbit());
  s2.reserve(150);
  for (size_t i = 71; i < s2.capacity(); i++)
    EXPECT_EQ(s2.rbit(), false);
}

TEST(Bitmask, CountTrue)
{
  auto m1 = Mask(64);
  for (size_t i = 0; i < 5; i++)
    m1.wtrue(i * 4);
  EXPECT_EQ(m1.count_true(), 5);

  m1.reset();
  m1.resize(110);
  for (size_t i = 0; i < 20; i++)
    m1.wtrue(i * 5);
  EXPECT_EQ(m1.count_true(), 20);

  m1.resize(60);
  EXPECT_EQ(m1.count_true(), 12);  // 0, 5, ..., 50, 55

  m1.resize(192);
  EXPECT_EQ(m1.count_true(), 13);  // 0, 5, ..., 55, 60

  for (size_t i = 0; i < 23; i++)
    m1.wtrue(i * 7);
  EXPECT_EQ(m1.count_true(), 34);  // 13 + 23 minus positions 0, 35.
}

TEST(Bitmask, RandomReadWrite)
{
  const size_t N = 192;
  auto m1 = Mask(N);
  EXPECT_EQ(m1.size(), N);
  m1.wlong(0, 928798ul);
  m1.wlong(64, 9845932ul);
  m1.wlong(128, 77719821ul);
  EXPECT_EQ(m1.rlong(1), 928798ul);
  EXPECT_EQ(m1.rlong(65), 9845932ul);
  EXPECT_EQ(m1.rlong(129), 77719821ul);

  auto vec = std::vector<bool>();
  for (size_t i = 0; i < N; i++)
    vec.push_back(m1.rbit(i));

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib(0, 1);
  for (size_t i = 30; i < N - 20; i++) {
    bool ran = distrib(gen);
    m1.wbit(i, ran);
    vec[i] = ran;
  }
  for (size_t i = 1; i < N; i += 35) {
    if (i % 2 == 0) {
      m1.wtrue(i);
      vec[i] = true;
    }
    else {
      m1.wfalse(i);
      vec[i] = false;
    }
  }
  for (size_t i = 0; i < N; i++)
    EXPECT_EQ(m1.rbit(i), vec[i]) << "at idx = " << i;
}

TEST(Bitmask, BufferTransfer)
{
  auto src = Mask(60);
  src.wlong(0, 78344ul);
  auto buf = src.view_buffer();

  auto dst = Mask(60);
  dst.use_bitstream(buf.data());
  EXPECT_EQ(src.size(), dst.size());
  for (size_t i = 0; i < src.size(); i++)
    EXPECT_EQ(src.rbit(i), dst.rbit(i));

  src.resize(120);
  src.wlong(100, 19837ul);
  buf = src.view_buffer();

  dst.resize(120);
  dst.use_bitstream(buf.data());
  EXPECT_EQ(src.size(), dst.size());
  for (size_t i = 0; i < src.size(); i++)
    EXPECT_EQ(src.rbit(i), dst.rbit(i));

  src.resize(128);
  buf = src.view_buffer();

  dst.resize(128);
  dst.use_bitstream(buf.data());
  EXPECT_EQ(src.size(), dst.size());
  for (size_t i = 0; i < src.size(); i++)
    EXPECT_EQ(src.rbit(i), dst.rbit(i));

  src.resize(150);
  src.wlong(130, 19837ul);
  buf = src.view_buffer();

  dst.resize(150);
  dst.use_bitstream(buf.data());
  EXPECT_EQ(src.size(), dst.size());
  for (size_t i = 0; i < src.size(); i++)
    EXPECT_EQ(src.rbit(i), dst.rbit(i));
}

TEST(Bitmask, has_true)
{
  const size_t mask_size = 210;

  // Loop over all positions
  for (size_t idx = 0; idx < mask_size; idx++) {
    auto mask = Mask(mask_size);
    mask.wtrue(idx);

    // Loop over all starting positions
    for (size_t start = 0; start < mask_size; start++) {

      // Loop over all range length
      for (size_t len = 0; len < mask_size - start; len++) {
        auto ans1 = mask.has_true<false>(start, len);
        auto ans2 = -1l;
        for (size_t i = start; i < start + len; i++)
          if (mask.rbit(i)) {
            ans2 = 1;
            break;
          }
        EXPECT_EQ(ans1, ans2);
      }

    }
  }
}

TEST(Bitmask, has_true_position)
{
  const size_t mask_size = 210;

  // Loop over all positions
  for (size_t idx = 0; idx < mask_size; idx++) {
    auto mask = Mask(mask_size);
    mask.wtrue(idx);

    // Loop over all starting positions
    for (size_t start = 0; start < mask_size; start++) {

      // Loop over all range length
      for (size_t len = 0; len < mask_size - start; len++) {
        auto ans1 = mask.has_true<true>(start, len);
        auto ans2 = -1l;
        for (size_t i = start; i < start + len; i++)
          if (mask.rbit(i)) {
            ans2 = i - start;
            break;
          }
        EXPECT_EQ(ans1, ans2) << "idx = " << idx << ", start = " << start << ", len = " << len << std::endl;
        if (ans1 != ans2)
          goto END_LABEL;
      }

    }
  }

END_LABEL:
  {}
}

#if __cplusplus >= 201907L && defined __cpp_lib_three_way_comparison
TEST(Bitmask, spaceship)
{
  auto src = Mask(60);
  auto dst = Mask(90);
  EXPECT_NE(src, dst);
  src.resize(90);
  EXPECT_EQ(src, dst);

  dst.wlong(64, std::numeric_limits<uint64_t>::max());
  EXPECT_NE(src, dst);

  for (size_t i = 64; i < 90; i++) {
    src.wbit(i, i % 3 == 0);
    dst.wbit(i, i % 3 == 0);
  }
  EXPECT_EQ(src.rlong(63), dst.rlong(63));
  EXPECT_NE(src.rlong(64), dst.rlong(64));
  EXPECT_EQ(src, dst);
}
#endif

}  // namespace
