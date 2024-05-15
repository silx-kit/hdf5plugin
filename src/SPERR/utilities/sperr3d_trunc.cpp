#include "SPERR3D_OMP_D.h"
#include "SPERR3D_Stream_Tools.h"

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

int main(int argc, char* argv[])
{
  // Parse command line options
  CLI::App app(
      "Truncate a SPERR3D bitstream to keep a percentage of its original length.\n"
      "Optionally, it can also evaluate the compression quality after truncation.\n");

  // Input specification
  auto input_file = std::string();
  app.add_option("filename", input_file, "The original SPERR3D bitstream to be truncated.\n")
      ->check(CLI::ExistingFile);

  //
  // Truncation settings
  //
  auto pct = uint32_t{0};
  app.add_option("--pct", pct, "Percentage (1--100) of the original bitstream to truncate.")
      ->required()
      ->group("Truncation settings");

  auto omp_num_threads = size_t{0};  // meaning to use the maximum number of threads.
#ifdef USE_OMP
  app.add_option("--omp", omp_num_threads,
                 "Number of OpenMP threads to use. Default (or 0) to use all.")
      ->group("Truncation settings");
#endif

  //
  // Output settings
  //
  auto out_file = std::string();
  app.add_option("-o", out_file, "Write out the truncated bitstream.")->group("Output settings");

  //
  // Input settings
  //
  auto orig32_file = std::string();
  app.add_option("--orig32", orig32_file,
                 "Original raw data in 32-bit precision to calculate compression\n"
                 "quality using the truncated bitstream.")
      ->group("Input settings");
  auto orig64_file = std::string();
  app.add_option("--orig64", orig64_file,
                 "Original raw data in 64-bit precision to calculate compression\n"
                 "quality using the truncated bitstream.")
      ->group("Input settings");

  CLI11_PARSE(app, argc, argv);

  //
  // A little sanity check
  //
  if (!orig32_file.empty() && !orig64_file.empty()) {
    std::cout << "Is the original data in 32 or 64 bit precision?" << std::endl;
    return __LINE__;
  }

  //
  // Really starting the real work!
  //
  auto tool = sperr::SPERR3D_Stream_Tools();
  auto stream_trunc = tool.progressive_read(input_file, pct);
  if (stream_trunc.empty()) {
    std::cout << "Error while truncating bitstream " << input_file << std::endl;
    return __LINE__;
  }
  auto header = tool.get_stream_header(stream_trunc.data());
  auto total_vals = header.vol_dims[0] * header.vol_dims[1] * header.vol_dims[2];
  auto real_bpp = stream_trunc.size() * 8.0 / double(total_vals);
  std::printf("Truncation resulting BPP = %.2f\n", real_bpp);

  if (!out_file.empty()) {
    auto rtn = sperr::write_n_bytes(out_file, stream_trunc.size(), stream_trunc.data());
    if (rtn != sperr::RTNType::Good)
      return __LINE__;
  }

  //
  // Decompress the truncated bitstream and evaluate compression quality.
  //
  if (!orig32_file.empty() || !orig64_file.empty()) {
    auto decoder = std::make_unique<sperr::SPERR3D_OMP_D>();
    decoder->set_num_threads(omp_num_threads);
    decoder->use_bitstream(stream_trunc.data(), stream_trunc.size());
    auto rtn = decoder->decompress(stream_trunc.data());
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Decompression failed!" << std::endl;
      return __LINE__;
    }
    auto outputd = decoder->release_decoded_data();
    decoder.reset();
    stream_trunc.clear();
    stream_trunc.shrink_to_fit();

    double linfy = 0.0, psnr = 0.0, rmse = 0.0, sigma = 0.0;

    if (!orig64_file.empty()) {
      auto orig = sperr::read_whole_file<double>(orig64_file);
      if (orig.empty()) {
        std::cout << "Read original data failed: " << orig64_file << std::endl;
        return __LINE__;
      }
      auto stats = sperr::calc_stats(orig.data(), outputd.data(), total_vals, omp_num_threads);
      auto mean_var = sperr::calc_mean_var(orig.data(), total_vals, omp_num_threads);
      rmse = stats[0];
      linfy = stats[1];
      psnr = stats[2];
      sigma = std::sqrt(mean_var[1]);
    }
    else {
      auto outputf = std::vector<float>(total_vals);
      std::copy(outputd.begin(), outputd.end(), outputf.begin());
      outputd.clear();
      outputd.shrink_to_fit();

      auto orig = sperr::read_whole_file<float>(orig32_file);
      if (orig.empty()) {
        std::cout << "Read original data failed: " << orig32_file << std::endl;
        return __LINE__;
      }
      auto stats = sperr::calc_stats(orig.data(), outputf.data(), total_vals, omp_num_threads);
      auto mean_var = sperr::calc_mean_var(orig.data(), total_vals, omp_num_threads);
      rmse = stats[0];
      linfy = stats[1];
      psnr = stats[2];
      sigma = std::sqrt(mean_var[1]);
    }

    std::printf("PSNR = %.2f, L-Infty = %.2e, Accuracy Gain = %.2f\n", psnr, linfy,
                std::log2(sigma / rmse) - real_bpp);
  }

  return 0;
}
