#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include "Outlier_Coder.h"
#include "gtest/gtest.h"

namespace {

using sperr::RTNType;

class outlier_tester {
 private:
  const size_t length = 0;
  const double tolerance = 0.0;

  std::vector<sperr::Outlier> LOS, recovered;

 public:
  // Constructor
  outlier_tester(size_t l, double t) : length(l), tolerance(t){};

  // A method to generate `N` outliers
  // The resulting outliers will be stored in `LOS`, and returned.
  auto gen_outliers(size_t N) -> const std::vector<sperr::Outlier>&
  {
    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution<double> val_d{0.0, tolerance};
    std::uniform_int_distribution<size_t> loc_d{0, length - 1};

    LOS.clear();
    LOS.reserve(N);
    while (LOS.size() < N) {
      double val = 0.0;
      while (std::abs(val) <= tolerance)
        val = val_d(gen);

      // Make sure there ain't duplicate locations
      auto loc = loc_d(gen);
      while (std::any_of(LOS.begin(), LOS.end(), [loc](auto out) { return out.pos == loc; })) {
        loc = loc_d(gen);
      }
      LOS.emplace_back(loc, val);
    }

    std::sort(LOS.begin(), LOS.end(), [](auto a, auto b) { return a.pos < b.pos; });
    return LOS;
  }

  auto test_outliers() -> const std::vector<sperr::Outlier>&
  {
    // Create an encoder
    sperr::Outlier_Coder encoder;
    encoder.set_length(length);
    encoder.set_tolerance(tolerance);
    encoder.use_outlier_list(LOS);

    if (encoder.encode() != RTNType::Good)
      return recovered;
    auto stream = sperr::vec8_type();
    encoder.append_encoded_bitstream(stream);

    // Create a decoder
    sperr::Outlier_Coder decoder;
    decoder.set_length(length);
    decoder.set_tolerance(tolerance);
    if (decoder.use_bitstream(stream.data(), stream.size()) != RTNType::Good)
      return recovered;
    if (decoder.decode() != RTNType::Good)
      return recovered;

    recovered = decoder.view_outlier_list();
    std::sort(recovered.begin(), recovered.end(), [](auto a, auto b) { return a.pos < b.pos; });
    return recovered;
  }
};

TEST(sperr, small_num_outliers)
{
  const double tolerance = 0.5;
  outlier_tester tester(10, tolerance);
  const auto& orig = tester.gen_outliers(3);
  const auto& recovered = tester.test_outliers();
  EXPECT_EQ(orig.size(), recovered.size());
  for (size_t i = 0; i < orig.size(); i++) {
    EXPECT_EQ(orig[i].pos, recovered[i].pos);
    EXPECT_NEAR(orig[i].err, recovered[i].err, tolerance);
  }
}

TEST(sperr, mid_num_outliers)
{
  const double tolerance = 3e-4;
  outlier_tester tester(10'000, tolerance);
  const auto& orig = tester.gen_outliers(190);
  const auto& recovered = tester.test_outliers();
  EXPECT_EQ(orig.size(), recovered.size());
  for (size_t i = 0; i < orig.size(); i++) {
    EXPECT_EQ(orig[i].pos, recovered[i].pos);
    EXPECT_NEAR(orig[i].err, recovered[i].err, tolerance);
  }
}

TEST(sperr, large_num_outliers)
{
  const double tolerance = 1e-7;
  outlier_tester tester(900'000, tolerance);
  const auto& orig = tester.gen_outliers(3900);
  const auto& recovered = tester.test_outliers();
  EXPECT_EQ(orig.size(), recovered.size());
  for (size_t i = 0; i < orig.size(); i++) {
    EXPECT_EQ(orig[i].pos, recovered[i].pos);
    EXPECT_NEAR(orig[i].err, recovered[i].err, tolerance);
  }
}

}  // namespace
