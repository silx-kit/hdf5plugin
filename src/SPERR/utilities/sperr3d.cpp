#include "SPERR3D_OMP_C.h"
#include "SPERR3D_OMP_D.h"

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

// This functions takes in a filename, and a full resolution. It then creates a list of
// filenames, each has the coarsened resolution appended.
auto create_filenames(std::string name,
                      sperr::dims_type vdims,
                      sperr::dims_type cdims) -> std::vector<std::string>
{
  auto filenames = std::vector<std::string>();
  auto resolutions = sperr::coarsened_resolutions(vdims, cdims);
  filenames.reserve(resolutions.size());
  for (auto res : resolutions)
    filenames.push_back(name + "." + std::to_string(res[0]) + "x" + std::to_string(res[1]) + "x" +
                        std::to_string(res[2]));

  return filenames;
}

// This function is used to output coarsened levels of the resolution hierarchy.
auto output_hierarchy(const std::vector<std::vector<double>>& hierarchy,
                      sperr::dims_type vdims,
                      sperr::dims_type cdims,
                      const std::string& lowres_f64,
                      const std::string& lowres_f32) -> int
{
  // If specified, output the low-res decompressed slices in double precision.
  if (!lowres_f64.empty()) {
    auto filenames = create_filenames(lowres_f64, vdims, cdims);
    assert(hierarchy.size() == filenames.size());
    for (size_t i = 0; i < filenames.size(); i++) {
      const auto& level = hierarchy[i];
      auto rtn = sperr::write_n_bytes(filenames[i], level.size() * sizeof(double), level.data());
      if (rtn != sperr::RTNType::Good) {
        std::cout << "Writing decompressed hierarchy failed: " << filenames[i] << std::endl;
        return __LINE__;
      }
    }
  }

  // If specified, output the low-res decompressed slices in single precision.
  if (!lowres_f32.empty()) {
    auto filenames = create_filenames(lowres_f32, vdims, cdims);
    assert(hierarchy.size() == filenames.size());
    auto buf = std::vector<float>(hierarchy.back().size());
    for (size_t i = 0; i < filenames.size(); i++) {
      const auto& level = hierarchy[i];
      std::copy(level.begin(), level.end(), buf.begin());
      auto rtn = sperr::write_n_bytes(filenames[i], level.size() * sizeof(float), buf.data());
      if (rtn != sperr::RTNType::Good) {
        std::cout << "Writing decompressed hierarchy failed: " << filenames[i] << std::endl;
        return __LINE__;
      }
    }
  }

  return 0;
}

// This function is used to output the decompressed volume at its native resolution.
auto output_buffer(const sperr::vecd_type& buf,
                   const std::string& name_f64,
                   const std::string& name_f32) -> int
{
  // If specified, output the decompressed slice in double precision.
  if (!name_f64.empty()) {
    auto rtn = sperr::write_n_bytes(name_f64, buf.size() * sizeof(double), buf.data());
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Writing decompressed data failed: " << name_f64 << std::endl;
      return __LINE__;
    }
  }

  // If specified, output the decompressed slice in single precision.
  if (!name_f32.empty()) {
    auto outputf = sperr::vecf_type(buf.size());
    std::copy(buf.cbegin(), buf.cend(), outputf.begin());
    auto rtn = sperr::write_n_bytes(name_f32, outputf.size() * sizeof(float), outputf.data());
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Writing decompressed data failed: " << name_f32 << std::endl;
      return __LINE__;
    }
  }

  return 0;
}

int main(int argc, char* argv[])
{
  // Parse command line options
  CLI::App app("3D SPERR compression and decompression\n");

  // Input specification
  auto input_file = std::string();
  app.add_option("filename", input_file,
                 "A data volume to be compressed, or\n"
                 "a bitstream to be decompressed.")
      ->check(CLI::ExistingFile);

  //
  // Execution settings
  //
  auto cflag = bool{false};
  auto* cptr =
      app.add_flag("-c", cflag, "Perform a compression task.")->group("Execution settings");
  auto dflag = bool{false};
  auto* dptr = app.add_flag("-d", dflag, "Perform a decompression task.")
                   ->excludes(cptr)
                   ->group("Execution settings");

  auto omp_num_threads = size_t{0};  // meaning to use the maximum number of threads.
#ifdef USE_OMP
  app.add_option("--omp", omp_num_threads,
                 "Number of OpenMP threads to use. Default (or 0) to use all.")
      ->group("Execution settings");
#endif

  //
  // Input properties
  //
  auto ftype = size_t{0};
  app.add_option("--ftype", ftype, "Specify the input float type in bits. Must be 32 or 64.")
      ->group("Input properties (for compression)");

  auto dims = std::array<size_t, 3>{0, 0, 0};
  app.add_option("--dims", dims,
                 "Dimensions of the input volume. E.g., `--dims 128 128 128`\n"
                 "(The fastest-varying dimension appears first.)")
      ->group("Input properties (for compression)");

  //
  // Output settings
  //
  auto bitstream = std::string();
  app.add_option("--bitstream", bitstream, "Output compressed bitstream.")
      ->needs(cptr)
      ->group("Output settings");

  auto decomp_f32 = std::string();
  app.add_option("--decomp_f", decomp_f32, "Output decompressed volume in f32 precision.")
      ->group("Output settings");

  auto decomp_f64 = std::string();
  app.add_option("--decomp_d", decomp_f64, "Output decompressed volume in f64 precision.")
      ->group("Output settings");

  auto decomp_lowres_f32 = std::string();
  app.add_option("--decomp_lowres_f", decomp_lowres_f32,
                 "Output lower resolutions of the decompressed volume in f32 precision.")
      ->group("Output settings");

  auto decomp_lowres_f64 = std::string();
  app.add_option("--decomp_lowres_d", decomp_lowres_f64,
                 "Output lower resolutions of the decompressed volume in f64 precision.")
      ->group("Output settings");

  auto print_stats = bool{false};
  app.add_flag("--print_stats", print_stats, "Print statistics measuring the compression quality.")
      ->needs(cptr)
      ->group("Output settings");

  //
  // Compression settings
  //
  auto chunks = std::array<size_t, 3>{256, 256, 256};
  app.add_option("--chunks", chunks,
                 "Dimensions of the preferred chunk size. Default: 256 256 256\n"
                 "(Volume dims don't need to be divisible by these chunk dims.)")
      ->group("Compression settings");

  auto pwe = 0.0;
  auto* pwe_ptr = app.add_option("--pwe", pwe, "Maximum point-wise error (PWE) tolerance.")
                      ->group("Compression settings");

  auto psnr = 0.0;
  auto* psnr_ptr = app.add_option("--psnr", psnr, "Target PSNR to achieve.")
                       ->excludes(pwe_ptr)
                       ->group("Compression settings");

  auto bpp = 0.0;
  auto* bpp_ptr = app.add_option("--bpp", bpp, "Target bit-per-pixel (bpp) to achieve.")
                      ->check(CLI::Range(0.0, 64.0))
                      ->excludes(pwe_ptr)
                      ->excludes(psnr_ptr)
                      ->group("Compression settings");

#ifdef EXPERIMENTING
  auto direct_q = 0.0;
  auto* dq_ptr = app.add_option("--dq", direct_q, "Directly provide the quantization step size q.")
                     ->excludes(bpp_ptr)
                     ->excludes(pwe_ptr)
                     ->excludes(psnr_ptr)
                     ->group("Compression settings");
#endif

  CLI11_PARSE(app, argc, argv);

  //
  // A little extra sanity check.
  //
  if (input_file.empty()) {
    std::cout << "What's the input file?" << std::endl;
    return __LINE__;
  }
  if (!cflag && !dflag) {
    std::cout << "Is this compressing (-c) or decompressing (-d) ?" << std::endl;
    return __LINE__;
  }
  if (cflag && dims == std::array{0ul, 0ul, 0ul}) {
    std::cout << "What's the dimensions of this 3D volume (--dims) ?" << std::endl;
    return __LINE__;
  }
  if (cflag && ftype != 32 && ftype != 64) {
    std::cout << "What's the floating-type precision (--ftype) ?" << std::endl;
    return __LINE__;
  }
#ifndef EXPERIMENTING
  if (cflag && pwe == 0.0 && psnr == 0.0 && bpp == 0.0) {
    std::cout << "What's the compression quality (--psnr, --pwe, --bpp) ?" << std::endl;
    return __LINE__;
  }
#endif
  if (cflag && (pwe < 0.0 || psnr < 0.0)) {
    std::cout << "Compression quality (--psnr, --pwe) must be positive!" << std::endl;
    return __LINE__;
  }
  if (dflag && decomp_f32.empty() && decomp_f64.empty() && decomp_lowres_f32.empty() &&
      decomp_lowres_f64.empty()) {
    std::cout << "SPERR needs an output destination when decoding!" << std::endl;
    return __LINE__;
  }
  // Also check if the chunk dims can support multi-resolution decoding.
  if (cflag && (!decomp_lowres_f64.empty() || !decomp_lowres_f32.empty())) {
    auto name = decomp_lowres_f64;
    if (name.empty())
      name = decomp_lowres_f32;
    assert(!name.empty());
    auto filenames = create_filenames(name, dims, chunks);
    if (filenames.empty()) {
      std::printf(
          " Warning: the combo of volume dimension (%lu, %lu, %lu) and chunk dimension"
          " (%lu, %lu, %lu)\n cannot support multi-resolution decoding. "
          " Try to use chunk dimensions that\n are similar in length and"
          " can divide the volume dimension.\n",
          dims[0], dims[1], dims[2], chunks[0], chunks[1], chunks[2]);
      return __LINE__ % 256;
    }
  }
  // Print a warning message if there's no output specified
  if (cflag && bitstream.empty())
    std::cout << "Warning: no output file provided. Consider using --bitstream option."
              << std::endl;
  if (dflag && decomp_f64.empty() && decomp_f32.empty() && decomp_lowres_f64.empty() &&
      decomp_lowres_f32.empty())
    std::cout << "Warning: no output file provided." << std::endl;

  //
  // Really starting the real work!
  //
  auto input = sperr::read_whole_file<uint8_t>(input_file);
  if (cflag) {
    const auto total_vals = dims[0] * dims[1] * dims[2];
    if ((ftype == 32 && (total_vals * 4 != input.size())) ||
        (ftype == 64 && (total_vals * 8 != input.size()))) {
      std::cout << "Input file size wrong!" << std::endl;
      return __LINE__ % 256;
    }
    auto encoder = std::make_unique<sperr::SPERR3D_OMP_C>();
    encoder->set_dims_and_chunks(dims, chunks);
    encoder->set_num_threads(omp_num_threads);
    if (pwe != 0.0)
      encoder->set_tolerance(pwe);
    else if (psnr != 0.0)
      encoder->set_psnr(psnr);
#ifdef EXPERIMENTING
    else if (direct_q != 0)
      encoder->set_direct_q(direct_q);
#endif
    else {
      assert(bpp != 0.0);
      encoder->set_bitrate(bpp);
    }

    auto rtn = sperr::RTNType::Good;
    if (ftype == 32)
      rtn = encoder->compress(reinterpret_cast<const float*>(input.data()), total_vals);
    else
      rtn = encoder->compress(reinterpret_cast<const double*>(input.data()), total_vals);
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Compression failed!" << std::endl;
      return __LINE__ % 256;
    }

    // If not calculating stats, we can free up some memory now!
    if (!print_stats) {
      input.clear();
      input.shrink_to_fit();
    }

    auto stream = encoder->get_encoded_bitstream();
    encoder.reset();  // Free up some more memory.

    // Output the compressed bitstream (maybe).
    if (!bitstream.empty()) {
      rtn = sperr::write_n_bytes(bitstream, stream.size(), stream.data());
      if (rtn != sperr::RTNType::Good) {
        std::cout << "Writing compressed bitstream failed: " << bitstream << std::endl;
        return __LINE__ % 256;
      }
    }

    //
    // Need to do a decompression in the following cases.
    //
    const auto multi_res = (!decomp_lowres_f32.empty()) || (!decomp_lowres_f64.empty());
    if (print_stats || !decomp_f64.empty() || !decomp_f32.empty() || multi_res) {
      auto decoder = std::make_unique<sperr::SPERR3D_OMP_D>();
      decoder->set_num_threads(omp_num_threads);
      decoder->use_bitstream(stream.data(), stream.size());
      rtn = decoder->decompress(stream.data(), multi_res);
      if (rtn != sperr::RTNType::Good) {
        std::cout << "Decompression failed!" << std::endl;
        return __LINE__ % 256;
      }

      // Save the decompressed data, and then deconstruct the decoder to free up some memory!
      auto outputd = decoder->release_decoded_data();
      auto hierarchy = decoder->release_hierarchy();
      decoder.reset();

      // Output the hierarchy (maybe), and then destroy it.
      auto ret = output_hierarchy(hierarchy, dims, chunks, decomp_lowres_f64, decomp_lowres_f32);
      if (ret)
        return __LINE__ % 256;
      hierarchy.clear();
      hierarchy.shrink_to_fit();

      // Output the decompressed volume (maybe).
      ret = output_buffer(outputd, decomp_f64, decomp_f32);
      if (ret)
        return __LINE__ % 256;

      // Calculate statistics.
      if (print_stats) {
        const double print_bpp = stream.size() * 8.0 / total_vals;
        double rmse, linfy, print_psnr, min, max, sigma;
        if (ftype == 32) {
          const float* inputf = reinterpret_cast<const float*>(input.data());
          auto outputf = sperr::vecf_type(total_vals);
          std::copy(outputd.cbegin(), outputd.cend(), outputf.begin());
          auto stats = sperr::calc_stats(inputf, outputf.data(), total_vals, omp_num_threads);
          rmse = stats[0];
          linfy = stats[1];
          print_psnr = stats[2];
          min = stats[3];
          max = stats[4];
          auto mean_var = sperr::calc_mean_var(inputf, total_vals, omp_num_threads);
          sigma = std::sqrt(mean_var[1]);
        }
        else {
          const double* inputd = reinterpret_cast<const double*>(input.data());
          auto stats = sperr::calc_stats(inputd, outputd.data(), total_vals, omp_num_threads);
          rmse = stats[0];
          linfy = stats[1];
          print_psnr = stats[2];
          min = stats[3];
          max = stats[4];
          auto mean_var = sperr::calc_mean_var(inputd, total_vals, omp_num_threads);
          sigma = std::sqrt(mean_var[1]);
        }
        std::printf("Input range = (%.2e, %.2e), L-Infty = %.2e\n", min, max, linfy);
        std::printf("Bitrate = %.2f, PSNR = %.2fdB, Accuracy Gain = %.2f\n", print_bpp, print_psnr,
                    std::log2(sigma / rmse) - print_bpp);
      }
    }
  }
  //
  // Decompression
  //
  else {
    assert(dflag);
    auto decoder = std::make_unique<sperr::SPERR3D_OMP_D>();
    decoder->set_num_threads(omp_num_threads);
    decoder->use_bitstream(input.data(), input.size());
    const auto multi_res = (!decomp_lowres_f32.empty()) || (!decomp_lowres_f64.empty());
    auto rtn = decoder->decompress(input.data(), multi_res);
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Decompression failed!" << std::endl;
      return __LINE__ % 256;
    }

    auto hierarchy = decoder->release_hierarchy();
    auto outputd = decoder->release_decoded_data();
    auto vdims = decoder->get_dims();
    auto cdims = decoder->get_chunk_dims();
    decoder.reset();  // Free up memory!

    // Output the hierarchy (maybe), and then destroy it.
    auto ret = output_hierarchy(hierarchy, vdims, cdims, decomp_lowres_f64, decomp_lowres_f32);
    if (ret)
      return __LINE__ % 256;
    hierarchy.clear();
    hierarchy.shrink_to_fit();

    // Output the decompressed volume (maybe).
    ret = output_buffer(outputd, decomp_f64, decomp_f32);
    if (ret)
      return __LINE__ % 256;
  }

  return 0;
}
