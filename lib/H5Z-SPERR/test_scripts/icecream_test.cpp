#include "gtest/gtest.h"
#include <vector>
#include <memory>
#include <random>
#include <cstring> // std::memcpy()

#include "icecream.h"

namespace {

TEST(icecream, StreamWriteRead)
{
  const size_t N = 159;
  auto mem = std::make_unique<uint64_t[]>(4);
  auto s1 = icecream();
  icecream_use_mem(&s1, mem.get(), 4);
  auto vec = std::vector<bool>(N);

  // Make N writes
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib1(0, 1);
  for (size_t i = 0; i < N; i++) {
    const auto bit = distrib1(gen);
    vec[i] = bit;
    EXPECT_EQ(icecream_wtell(&s1), i) << " at idx = " << i;
    icecream_wbit(&s1, bit);
    EXPECT_EQ(icecream_wtell(&s1), i + 1) << " at idx = " << i;
  }
  EXPECT_EQ(icecream_wtell(&s1), N);
  icecream_flush(&s1);
  EXPECT_EQ(icecream_wtell(&s1), 192);

  icecream_rewind(&s1);
  for (size_t i = 0; i < N; i++) {
    EXPECT_EQ(icecream_rtell(&s1), i) << " at idx = " << i;
    EXPECT_EQ(icecream_rbit(&s1), vec[i]) << " at idx = " << i;
    EXPECT_EQ(icecream_rtell(&s1), i + 1) << " at idx = " << i;
  }
}

TEST(icecream, PartialWord)
{
  auto mem = std::make_unique<char[]>(20);
  auto s1 = icecream();
  icecream_use_mem(&s1, mem.get(), 20);
  auto vec = std::vector<bool>();

  // Make 80 writes. The first 10 bytes are supposed to keep the result.
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<unsigned int> distrib(0, 1);
  for (int i = 0; i < 80; i++) {
    const auto bit = distrib(gen);
    vec.push_back(bit);
    icecream_wbit(&s1, bit);
  }
  icecream_flush(&s1);

  // Copy over the first 10 bytes, and test if the bits are the same.
  auto mem2 = std::make_unique<char[]>(16);
  for (int i = 0; i < 16; i++)
    mem2[i] = 15;
  std::memcpy(mem2.get(), mem.get(), 10);
  icecream_use_mem(&s1, mem2.get(), 16);
  for (int i = 0; i < 80; i++)
    EXPECT_EQ(icecream_rbit(&s1), vec[i]) << " at idx = " << i;
}


}
