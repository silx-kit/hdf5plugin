#include <array>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

#define FLOAT double

//
// Given a big volume with dimension `dims` and logical indices  `ijk`,
// return a location in the 1D array representing the volume.
//
size_t translate_idx(std::array<size_t, 3> dims, std::array<size_t, 3> ijk)
{
  const auto plane = ijk[2] * dims[0] * dims[1];
  const auto col = ijk[1] * dims[0];
  return (ijk[0] + col + plane);
}

int main(int argc, char* argv[])
{
  if (argc != 12) {
    std::cerr << "Usage: InFileName, InFileNX, InFileNY, InFileNZ, "
              << "OutFileName, OutStartX, OutFinishX, "
              << "OutStartY, OutFinishY, OutStartZ, OutFinishZ. " << std::endl
              << "For example, if you want to crop from index 64 "
              << "to index 128 (exclusive) in X dimension, just type 64 and 128!" << std::endl;
    exit(1);
  }

  /*
   * It is the user's responsibility to make sure the range to crop
   * is within the range of input file.
   */
  auto inDims = std::array<size_t, 3>{static_cast<size_t>(std::atol(argv[2])),
                                      static_cast<size_t>(std::atol(argv[3])),
                                      static_cast<size_t>(std::atol(argv[4]))};
  auto inValNum = inDims[0] * inDims[1] * inDims[2];
  size_t outStartX = std::atol(argv[6]);
  size_t outFinishX = std::atol(argv[7]);
  size_t outStartY = std::atol(argv[8]);
  size_t outFinishY = std::atol(argv[9]);
  size_t outStartZ = std::atol(argv[10]);
  size_t outFinishZ = std::atol(argv[11]);
  auto outDims =
      std::array<size_t, 3>{outFinishX - outStartX, outFinishY - outStartY, outFinishZ - outStartZ};
  auto outValNum = outDims[0] * outDims[1] * outDims[2];

  std::FILE* infile = std::fopen(argv[1], "rb");
  if (infile == nullptr) {
    std::cerr << "input file open error: " << argv[1] << std::endl;
    exit(1);
  }

  auto inbuf = std::make_unique<FLOAT[]>(inValNum);
  auto cnt = std::fread(inbuf.get(), sizeof(FLOAT), inValNum, infile);
  if (cnt != inValNum) {
    std::cerr << "input file read error: " << argv[1] << std::endl;
    exit(1);
  }

  auto outbuf = std::make_unique<FLOAT[]>(outValNum);
  size_t counter = 0;
  for (size_t z = outStartZ; z < outFinishZ; z++) {
    for (size_t y = outStartY; y < outFinishY; y++) {
      for (size_t x = outStartX; x < outFinishX; x++) {
        auto idx = translate_idx(inDims, {x, y, z});
        outbuf[counter++] = inbuf[idx];
      }
    }
  }

  std::FILE* outfile = std::fopen(argv[5], "wb");
  if (outfile == nullptr) {
    std::cerr << "output file open error: " << argv[5] << std::endl;
    exit(1);
  }

  cnt = std::fwrite(outbuf.get(), sizeof(FLOAT), outValNum, outfile);
  if (cnt != outValNum) {
    std::cerr << "output file write error: " << argv[5] << std::endl;
    exit(1);
  }

  std::fclose(infile);
  std::fclose(outfile);

  return 0;
}
