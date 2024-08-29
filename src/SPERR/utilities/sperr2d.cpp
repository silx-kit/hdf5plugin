#include "SPECK2D_FLT.h"

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>

// This functions takes in a filename, and a full resolution. It then creates a list of
// filenames, each has the coarsened resolution appended.
auto create_filenames(std::string name, sperr::dims_type dims) -> std::vector<std::string>
{
  auto filenames = std::vector<std::string>();
  auto resolutions = sperr::coarsened_resolutions(dims);
  filenames.reserve(resolutions.size());
  for (auto res : resolutions)
    filenames.push_back(name + "." + std::to_string(res[0]) + "x" + std::to_string(res[1]));

  return filenames;
}

// This function is used to output coarsened levels of the resolution hierarchy.
auto output_hierarchy(const std::vector<std::vector<double>>& hierarchy,
                      sperr::dims_type dims,
                      const std::string& lowres_f64,
                      const std::string& lowres_f32) -> int
{
  // If specified, output the low-res decompressed slices in double precision.
  if (!lowres_f64.empty()) {
    auto filenames = create_filenames(lowres_f64, dims);
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
    auto filenames = create_filenames(lowres_f32, dims);
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

// This function is used to output the decompressed slice at its native resolution.
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
  CLI::App app("2D SPERR compression and decompression\n");

  // Input specification
  auto input_file = std::string();
  app.add_option("filename", input_file,
                 "A data slice to be compressed, or\n"
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

  //
  // Input properties
  //
  auto ftype = size_t{0};
  app.add_option("--ftype", ftype, "Specify the input float type in bits. Must be 32 or 64.")
      ->group("Input properties");

  auto dim2d = std::array<uint32_t, 2>{0, 0};
  app.add_option("--dims", dim2d,
                 "Dimensions of the input slice. E.g., `--dims 128 128`\n"
                 "(The fastest-varying dimension appears first.)")
      ->group("Input properties");

  //
  // Output settings
  //
  auto bitstream = std::string();
  app.add_option("--bitstream", bitstream, "Output compressed bitstream.")
      ->needs(cptr)
      ->group("Output settings");

  auto decomp_f32 = std::string();
  app.add_option("--decomp_f", decomp_f32, "Output decompressed slice in f32 precision.")
      ->group("Output settings");

  auto decomp_f64 = std::string();
  app.add_option("--decomp_d", decomp_f64, "Output decompressed slice in f64 precision.")
      ->group("Output settings");

  auto decomp_lowres_f32 = std::string();
  app.add_option("--decomp_lowres_f", decomp_lowres_f32,
                 "Output lower resolutions of the decompressed slice in f32 precision.")
      ->group("Output settings");

  auto decomp_lowres_f64 = std::string();
  app.add_option("--decomp_lowres_d", decomp_lowres_f64,
                 "Output lower resolutions of the decompressed slice in f64 precision.")
      ->group("Output settings");

  auto print_stats = bool{false};
  app.add_flag("--print_stats", print_stats, "Show statistics measuring the compression quality.")
      ->needs(cptr)
      ->group("Output settings");

  //
  // Compression settings
  //
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
  if (cflag && dim2d == std::array{uint32_t{0}, uint32_t{0}}) {
    std::cout << "What's the dimensions of this 2D slice (--dims) ?" << std::endl;
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
  // Print a warning message if there's no output specified
  if (cflag && bitstream.empty())
    std::cout << "Warning: no output file provided. Consider using --bitstream option."
              << std::endl;
  if (dflag && decomp_f64.empty() && decomp_f32.empty() && decomp_lowres_f64.empty() &&
      decomp_lowres_f32.empty())
    std::cout << "Warning: no output file provided." << std::endl;

  //
  // Really starting the real work!
  // Note: the header has the same format as in SPERR3D_OMP_C.
  //
  const auto header_len = 10ul;
  auto input = sperr::read_whole_file<uint8_t>(input_file);
  if (cflag) {
    const auto dims = sperr::dims_type{dim2d[0], dim2d[1], 1ul};
    const auto total_vals = dims[0] * dims[1] * dims[2];
    if ((ftype == 32 && (total_vals * 4 != input.size())) ||
        (ftype == 64 && (total_vals * 8 != input.size()))) {
      std::cout << "Input file size wrong!" << std::endl;
      return __LINE__ % 256;
    }
    auto encoder = std::make_unique<sperr::SPECK2D_FLT>();
    encoder->set_dims(dims);
    if (ftype == 32)
      encoder->copy_data(reinterpret_cast<const float*>(input.data()), total_vals);
    else
      encoder->copy_data(reinterpret_cast<const double*>(input.data()), total_vals);

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

    // If not calculating stats, we can free up some memory now!
    if (!print_stats) {
      input.clear();
      input.shrink_to_fit();
    }

    auto rtn = encoder->compress();
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Compression failed!" << std::endl;
      return __LINE__ % 256;
    }

    // Assemble the output bitstream.
    auto stream = sperr::vec8_type(header_len);
    stream[0] = static_cast<uint8_t>(SPERR_VERSION_MAJOR);
    const auto b8 = std::array{false,  // not a portion
                               false,  // 2D
                               ftype == 32,
                               false,   // unused
                               false,   // unused
                               false,   // unused
                               false,   // unused
                               false};  // unused
    stream[1] = sperr::pack_8_booleans(b8);
    std::memcpy(stream.data() + 2, dim2d.data(), sizeof(dim2d));
    encoder->append_encoded_bitstream(stream);
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
      auto decoder = std::make_unique<sperr::SPECK2D_FLT>();
      decoder->set_dims(dims);
      // !! Remember the header thing !!
      decoder->use_bitstream(stream.data() + header_len, stream.size() - header_len);
      rtn = decoder->decompress(multi_res);
      if (rtn != sperr::RTNType::Good) {
        std::cout << "Decompression failed!" << std::endl;
        return __LINE__ % 256;
      }

      // Save the decompressed data, and then deconstruct the decoder to free up some memory!
      auto hierarchy = decoder->release_hierarchy();
      auto outputd = decoder->release_decoded_data();
      decoder.reset();

      // Output the hierarchy (maybe), and then destroy it.
      auto ret = output_hierarchy(hierarchy, dims, decomp_lowres_f64, decomp_lowres_f32);
      if (ret)
        return __LINE__ % 256;
      hierarchy.clear();
      hierarchy.shrink_to_fit();

      // Output the decompressed slice (maybe).
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
          auto stats = sperr::calc_stats(inputf, outputf.data(), total_vals);
          rmse = stats[0];
          linfy = stats[1];
          print_psnr = stats[2];
          min = stats[3];
          max = stats[4];
          auto mean_var = sperr::calc_mean_var(inputf, total_vals);
          sigma = std::sqrt(mean_var[1]);
        }
        else {
          const double* inputd = reinterpret_cast<const double*>(input.data());
          auto stats = sperr::calc_stats(inputd, outputd.data(), total_vals);
          rmse = stats[0];
          linfy = stats[1];
          print_psnr = stats[2];
          min = stats[3];
          max = stats[4];
          auto mean_var = sperr::calc_mean_var(inputd, total_vals);
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

    if (input[0] != (SPERR_VERSION_MAJOR)) {
      std::cout << "This bitstream is produced by a compressor of a different version!"
                << std::endl;
      return __LINE__ % 256;
    }
    auto booleans = sperr::unpack_8_booleans(input[1]);
    if (booleans[1]) {
      std::cout << "This bitstream appears to represent a 3D volume!" << std::endl;
      return __LINE__ % 256;
    }

    // Retrieve the slice dimension from the header.
    std::memcpy(dim2d.data(), input.data() + 2, sizeof(dim2d));
    const auto dims = sperr::dims_type{dim2d[0], dim2d[1], 1ul};
    auto decoder = std::make_unique<sperr::SPECK2D_FLT>();
    decoder->set_dims(dims);
    decoder->use_bitstream(input.data() + header_len, input.size() - header_len);
    const auto multi_res = (!decomp_lowres_f32.empty()) || (!decomp_lowres_f64.empty());
    auto rtn = decoder->decompress(multi_res);
    if (rtn != sperr::RTNType::Good) {
      std::cout << "Decompression failed!" << std::endl;
      return __LINE__ % 256;
    }

    // Save the decompressed data, and then deconstruct the decoder to free up some memory!
    auto hierarchy = decoder->release_hierarchy();
    auto outputd = decoder->release_decoded_data();
    decoder.reset();

    // Output the hierarchy (maybe).
    auto ret = output_hierarchy(hierarchy, dims, decomp_lowres_f64, decomp_lowres_f32);
    if (ret)
      return __LINE__ % 256;

    // Free up the hierarchy.
    hierarchy.clear();
    hierarchy.shrink_to_fit();

    // Output the decompressed slice (maybe).
    ret = output_buffer(outputd, decomp_f64, decomp_f32);
    if (ret)
      return __LINE__ % 256;
  }

  return 0;
}
