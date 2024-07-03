#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#define FLOAT float

//
// Given a big volume with dimension `dims` and logical indices  `ijk`,
// return a location in the 1D array representing the volume.
//
size_t translate_idx(const std::array<size_t, 3>& dims, std::array<size_t, 3> ijk)
{
  const auto plane = ijk[2] * dims[0] * dims[1];
  const auto col = ijk[1] * dims[0];
  return (ijk[0] + col + plane);
}

int main(int argc, char* argv[])
{
  if (argc != 12) {
    std::cerr << "Usage: big_vol_name, big_vol_NX, big_vol_NY, big_vol_NZ, \n"
              << "small_vol_name, small_vol_nx, small_vol_ny, small_vol_nz, \n"
              << "put_at_big_x, Oput_at_big_y, put_at_big_z. " << std::endl;
    exit(1);
  }

  //
  // It is the user's responsibility to make sure the range and dimensions are
  // good.
  //
  auto big_dims = std::array<size_t, 3>{std::atol(argv[2]), std::atol(argv[3]), std::atol(argv[4])};
  auto small_dims =
      std::array<size_t, 3>{std::atol(argv[6]), std::atol(argv[7]), std::atol(argv[8])};
  auto big_n_vals = big_dims[0] * big_dims[1] * big_dims[2];
  auto small_n_vals = small_dims[0] * small_dims[1] * small_dims[2];
  size_t put_at_big_x = std::atol(argv[9]);
  size_t put_at_big_y = std::atol(argv[10]);
  size_t put_at_big_z = std::atol(argv[11]);

  //
  // Read in the big file
  //
  std::FILE* big_file = std::fopen(argv[1], "rb");
  if (big_file == nullptr) {
    std::cerr << "big file open error: " << argv[1] << std::endl;
    exit(1);
  }
  auto big_buf = std::make_unique<FLOAT[]>(big_n_vals);
  auto cnt = std::fread(big_buf.get(), sizeof(FLOAT), big_n_vals, big_file);
  if (cnt != big_n_vals) {
    std::cerr << "big file read error: " << argv[1] << std::endl;
    exit(1);
  }
  std::fclose(big_file);

  //
  // Read in the small file
  //
  std::FILE* small_file = std::fopen(argv[5], "rb");
  if (small_file == nullptr) {
    std::cerr << "small file open error: " << argv[5] << std::endl;
    exit(1);
  }
  auto small_buf = std::make_unique<FLOAT[]>(small_n_vals);
  cnt = std::fread(small_buf.get(), sizeof(FLOAT), small_n_vals, small_file);
  if (cnt != small_n_vals) {
    std::cerr << "small file read error: " << argv[5] << std::endl;
    exit(1);
  }
  std::fclose(small_file);

  //
  // Put values from the small file buffer to locations at the big file buffer.
  //
  size_t small_idx = 0;
  for (size_t z = put_at_big_z; z < put_at_big_z + small_dims[2]; z++) {
    for (size_t y = put_at_big_y; y < put_at_big_y + small_dims[1]; y++) {
      for (size_t x = put_at_big_x; x < put_at_big_x + small_dims[0]; x++) {
        auto big_idx = translate_idx(big_dims, {x, y, z});
        big_buf[big_idx] = small_buf[small_idx++];
      }
    }
  }

  //
  // Write back the big file buffer
  //
  big_file = std::fopen(argv[1], "wb");
  if (big_file == nullptr) {
    std::cerr << "big file open error: " << argv[1] << std::endl;
    exit(1);
  }
  cnt = std::fwrite(big_buf.get(), sizeof(FLOAT), big_n_vals, big_file);
  if (cnt != big_n_vals) {
    std::cerr << "output file write error: " << argv[5] << std::endl;
    exit(1);
  }
  std::fclose(big_file);

  return 0;
}
