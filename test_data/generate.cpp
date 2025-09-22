#include <algorithm>
#include <cstdio>
#include <cmath>
#include <memory>

int main()
{
#if 0
  // 2D case
  const long N = 100;
  const long N2 = N / 2;
  const long len = N * N;
  auto buf = std::make_unique<float[]>(len);

  long idx = 0;
  for (long y = 0; y < N; y++) {
    for (long x = 0; x < N; x++) {
      auto dist = std::sqrt(float((x - N2) * (x - N2) + (y - N2) * (y - N2)));
      buf[idx++] = 1.f / dist;
    }
  }

  // There's an inf produced at [N2, N2] position, so correct it here.
  buf[N2 * N + N2] = 1.f;

  auto* f = std::fopen( "buf.bin", "wb" );
  std::fwrite( buf.get(), sizeof(float), len, f );
  std::fclose( f );
#endif

  // 3D case
  const long N = 100;
  const long N2 = N / 2;
  const long len = N * N * N;
  auto buf = std::make_unique<float[]>(len);

  long idx = 0;
  for (long z = 0; z < N; z++) {
    for (long y = 0; y < N; y++) {
      for (long x = 0; x < N; x++) {
        auto dist = std::sqrt(float((x-N2) * (x-N2) + (y-N2) * (y-N2) + (z-N2) * (z-N2)));
        buf[idx++] = 1.f / dist;
      }
    }
  }

  // There's an inf produced at [N2, N2, N2] position, so correct it here.
  buf[N2 * N * N + N2 * N + N2] = 1.f;

  auto* f = std::fopen( "ball100.bin", "wb" );
  std::fwrite( buf.get(), sizeof(float), len, f );
  std::fclose( f );
}
