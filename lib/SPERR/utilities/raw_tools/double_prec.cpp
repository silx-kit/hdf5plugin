#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

union Double_t {
  Double_t(double val) : f(val) {}
  Double_t(int64_t val) : i(val) {}

  bool Negative() const { return (i >> 63) != 0; }
  int64_t RawMantissa() const { return i & ((int64_t(i) << 52) - 1); }
  int64_t RawExponent() const { return (i >> 52) & 0x7FF; }

  int64_t i;
  double f;

  struct {  // Bitfields for exploration. Do not use in production code.
    uint64_t mantissa : 52;
    uint64_t exponent : 11;
    uint64_t sign : 1;
  } parts;
};

int main(int argc, char* argv[])
{
  // The last double value that still has precision as good as int.
  // The next double value is 2.0 bigger.
  Double_t num(0x1.fffffffffffffp52);

  // Comment out the line below, then you'll notice that it's impossible to
  // increment by 1.0 on the double value!
  // num.i++;

  std::printf(
      "Float value,    int value,        storage in hex,     "
      "storage in dec,     sign, exponent, mantissa\n");
  std::printf("%1.8e, %ld, 0x%08lx, %ld,  %d, %d, 0x%06lx\n", num.f, std::lrint(num.f), num.i,
              num.i, num.parts.sign, num.parts.exponent, num.parts.mantissa);

  double d1 = num.f;
  num.i += 1;
  double d2 = num.f;
  std::cout << "after increment by 1, d2 - d1 = " << d2 - d1 << std::endl;
  num.i -= 2;
  d2 = num.f;
  std::cout << "after decrement by 1, d2 - d1 = " << d2 - d1 << std::endl;

  // std::cout << std::lrint(num.f) << std::endl;
  // std::cout << std::numeric_limits<int32_t>::max() << std::endl;
  // std::cout << std::numeric_limits<int64_t>::max() << std::endl;
}
