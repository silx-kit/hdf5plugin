#include "gtest/gtest.h"

#include <cmath>
#include <numeric>
#include <memory>

#include "h5zsperr_helper.h"
#include "compactor.h"
#include "icecream.h"

namespace {

TEST(h5zsperr_helper, pack_extra_info)
{
  // Test all possible combinations
  int rank = 0, is_float = 0, missing_val_mode = 0, magic = 0;
  for (int r = 2; r <= 3; r++)
    for (int f = 0; f <= 1; f++)
      for (int m = 0; m <= 2; m++)
        for (int g = 0; g <= 63; g++) {
          auto encode = C_API::h5zsperr_pack_extra_info(r, f, m, g);
          rank = r * f * m + 10;
          is_float = rank + 10;
          missing_val_mode = rank + 20;
          magic = rank - 20;
          C_API::h5zsperr_unpack_extra_info(encode, &rank, &is_float, &missing_val_mode, &magic);
          ASSERT_EQ(rank, r);
          ASSERT_EQ(is_float, f);
          ASSERT_EQ(missing_val_mode, m);
          ASSERT_EQ(magic, g);
      }
}

TEST(h5zsperr_helper, make_mask_nan1)
{
  // Create a float array with just a few NaNs.
  int N = 256;
  auto buf = std::vector<float>(N);
  for (int i = 0; i < N; i++)
    buf[i] = float(i);
  buf[50] = std::nanf("1");
  buf[90] = std::nanf("1");
  buf[200] = std::nanf("1");

  // Create a mask using std::vector<bool>.
  auto mask1 = std::vector<bool>(N);
  for (int i = 0; i < N; i++)
    mask1[i] = std::isnan(buf[i]);

  // Create a compact mask using the helper function.
  size_t nbytes = N / 8;
  auto mask2 = std::make_unique<char[]>(nbytes);
  size_t useful_bytes2 = 0;
  auto ret = C_API::h5zsperr_make_mask_nan(buf.data(), N, 1, mask2.get(), nbytes, &useful_bytes2);
  ASSERT_EQ(ret, 0);

  // Decode mask2
  auto mask3 = std::make_unique<char[]>(N);
  auto useful_bytes3 = compactor_decode(mask2.get(), nbytes, mask3.get());
  ASSERT_EQ(useful_bytes3, nbytes);

  // Test that mask3 equals mask1
  auto s1 = icecream();
  icecream_use_mem(&s1, mask3.get(), N);
  for (int i = 0; i < N; i++)
    ASSERT_EQ(mask1[i], icecream_rbit(&s1));
}

TEST(h5zsperr_helper, make_mask_nan2)
{
  // Create a float array with just a few non-NaNs.
  int N = 300;
  auto buf = std::vector<float>(N);
  for (int i = 0; i < N; i++)
    buf[i] = std::nanf("1");
  buf[50] = 50.f;
  buf[90] = 90.f;
  buf[200] = 200.f;
  buf[299] = 299.f;

  // Create a mask using std::vector<bool>.
  auto mask1 = std::vector<bool>(N);
  for (int i = 0; i < N; i++)
    mask1[i] = std::isnan(buf[i]);

  // Create a compact mask using the helper function.
  size_t nbytes = (N + 7) / 8;
  auto mask2 = std::make_unique<char[]>(nbytes);
  size_t useful_bytes2 = 0;
  auto ret = C_API::h5zsperr_make_mask_nan(buf.data(), N, 1, mask2.get(), nbytes, &useful_bytes2);
  ASSERT_EQ(ret, 0);

  // Decode mask2
  auto mask3 = std::make_unique<char[]>(N);
  while (useful_bytes2 % 8)
    useful_bytes2++;
  auto decoded_bytes3 = compactor_decode(mask2.get(), useful_bytes2, mask3.get());

  // Test that mask3 equals mask1
  auto s1 = icecream();
  icecream_use_mem(&s1, mask3.get(), N);
  for (int i = 0; i < N; i++)
    ASSERT_EQ(mask1[i], icecream_rbit(&s1));
}

TEST(h5zsperr_helper, treat_nan)
{
  size_t N = 131;
  auto buf = std::vector<float>(N);
  for (size_t i = 0; i < N; i++)
    buf[i] = float(i + 1) * 0.5f;
  buf[10] = 0.f;
  buf[20] = 0.f;
  buf[90] = 0.f;
  auto mean = std::accumulate(buf.begin(), buf.end(), 0.f) / 128.f;
  buf[10] = mean;
  buf[20] = mean;
  buf[90] = mean;

  auto buf2 = buf;
  buf2[10] = std::nanf("1");
  buf2[20] = std::nanf("1");
  buf2[90] = std::nanf("1");
  auto mean2 = C_API::h5zsperr_treat_nan_f32(buf2.data(), N);

  ASSERT_FLOAT_EQ(mean, mean2);
  for (size_t i = 0; i < N; i++)
    ASSERT_FLOAT_EQ(buf[i], buf2[i]) << "i = " << i;
}

TEST(h5zsperr_helper, treat_large_mag)
{
  size_t N = 120;
  auto buf = std::vector<double>(N);
  for (size_t i = 0; i < N; i++)
    buf[i] = double(i + 1) * 0.5;
  buf[10] = 0.0;
  buf[20] = 0.0;
  buf[90] = 0.0;
  auto mean = std::accumulate(buf.begin(), buf.end(), 0.0) / 117.f;
  buf[10] = mean;
  buf[20] = mean;
  buf[90] = mean;

  auto buf2 = buf;
  buf2[10] = LARGE_MAGNITUDE_D;
  buf2[20] = LARGE_MAGNITUDE_D;
  buf2[90] = LARGE_MAGNITUDE_D;
  auto tmp = C_API::h5zsperr_treat_large_mag_f64(buf2.data(), N);
  ASSERT_EQ(tmp, LARGE_MAGNITUDE_D);

  for (size_t i = 0; i < N; i++)
    ASSERT_DOUBLE_EQ(buf[i], buf2[i]) << "i = " << i;
}

}
