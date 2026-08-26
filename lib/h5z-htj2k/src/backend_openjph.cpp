#include "H5Zhtj2k_backend.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>

#include "ojph_arch.h"
#include "ojph_codestream.h"
#include "ojph_file.h"
#include "ojph_mem.h"
#include "ojph_params.h"

namespace {

struct DtypeInfo {
  ojph::ui32 precision;
  bool is_signed;
  size_t size;
};

bool dtype_info(unsigned int dtype, DtypeInfo &info) {
  const size_t nbytes = H5Z_HTJ2K_DTYPE_NBYTES(dtype);
  const bool is_signed = H5Z_HTJ2K_DTYPE_IS_SIGNED(dtype);
  if ((nbytes != 1 && nbytes != 2) ||
      dtype != static_cast<unsigned int>(H5Z_HTJ2K_DTYPE(is_signed, nbytes))) {
    return false;
  }
  info.precision = static_cast<ojph::ui32>(nbytes * 8);
  info.is_signed = is_signed;
  info.size = nbytes;
  return true;
}

/* dtype -> H5Z_HTJ2K_DTYPE_t for reconstructed samples, from
 * bit depth/signedness reported by the decoded codestream. */
bool resolve_output_dtype(ojph::ui32 precision, bool is_signed,
                          unsigned int *out_dtype, size_t *sample_size) {
  if (precision > 16) {
    return false;
  }
  *sample_size = (precision <= 8) ? 1 : 2;
  *out_dtype = H5Z_HTJ2K_DTYPE(is_signed, *sample_size);
  return true;
}

} // namespace

extern "C" int h5z_htj2k_backend_compress(
    size_t input_nbytes, void *input_buffer, unsigned int width,
    unsigned int height, unsigned int ncomps, unsigned int dtype,
    int num_threads, size_t *output_nbytes, void **output_buffer) {
  (void)num_threads; /* OpenJPH's core codec is single-threaded */

  if (!input_buffer || !output_nbytes || !output_buffer || width == 0 ||
      height == 0 || ncomps == 0) {
    return -1;
  }
  *output_nbytes = 0;
  *output_buffer = nullptr;

  DtypeInfo di;
  if (!dtype_info(dtype, di)) {
    fprintf(stderr, "[h5z-htj2k/OpenJPH] unsupported data type\n");
    return -1;
  }

  const size_t expected =
      static_cast<size_t>(width) * height * ncomps * di.size;
  if (input_nbytes != expected) {
    fprintf(stderr, "[h5z-htj2k/OpenJPH] input_nbytes mismatch: %zu != %zu\n",
            input_nbytes, expected);
    return -1;
  }

  try {
    ojph::codestream codestream;

    ojph::param_siz siz = codestream.access_siz();
    siz.set_image_extent(ojph::point(width, height));
    siz.set_num_components(ncomps);
    for (unsigned int c = 0; c < ncomps; c++)
      siz.set_component(c, ojph::point(1, 1), di.precision, di.is_signed);
    siz.set_image_offset(ojph::point(0, 0));
    siz.set_tile_size(ojph::size(0, 0));
    siz.set_tile_offset(ojph::point(0, 0));

    ojph::param_cod cod = codestream.access_cod();
    cod.set_num_decomposition(5);
    cod.set_block_dims(64, 64);
    cod.set_progression_order("RPCL");
    cod.set_color_transform(false); /* samples are not assumed to be RGB */
    cod.set_reversible(true);       /* lossless 5/3 wavelet */

    codestream.set_planar(false);

    ojph::mem_outfile file;
    file.open();
    codestream.write_headers(&file);

    ojph::ui32 next_comp;
    ojph::line_buf *cur_line = codestream.exchange(nullptr, next_comp);

    for (unsigned int y = 0; y < height; y++) {
      for (unsigned int c = 0; c < ncomps; c++) {
        ojph::si32 *dst = cur_line->i32;
        const size_t row_offset = (static_cast<size_t>(y) * width) * ncomps + c;
        switch (di.size) {
        case 1:
          if (di.is_signed) {
            const int8_t *src =
                static_cast<const int8_t *>(input_buffer) + row_offset;
            for (unsigned int x = 0; x < width; x++)
              dst[x] = src[static_cast<size_t>(x) * ncomps];
          } else {
            const uint8_t *src =
                static_cast<const uint8_t *>(input_buffer) + row_offset;
            for (unsigned int x = 0; x < width; x++)
              dst[x] = src[static_cast<size_t>(x) * ncomps];
          }
          break;
        case 2:
          if (di.is_signed) {
            const int16_t *src =
                static_cast<const int16_t *>(input_buffer) + row_offset;
            for (unsigned int x = 0; x < width; x++)
              dst[x] = src[static_cast<size_t>(x) * ncomps];
          } else {
            const uint16_t *src =
                static_cast<const uint16_t *>(input_buffer) + row_offset;
            for (unsigned int x = 0; x < width; x++)
              dst[x] = src[static_cast<size_t>(x) * ncomps];
          }
          break;
        }
        cur_line = codestream.exchange(cur_line, next_comp);
      }
    }

    codestream.flush();
    codestream.close();

    size_t used = file.get_used_size();
    void *out = std::malloc(used);
    if (!out) {
      return -1;
    }
    std::memcpy(out, file.get_data(), used);
    *output_buffer = out;
    *output_nbytes = used;
    return 0;
  } catch (const std::exception &e) {
    fprintf(stderr, "[h5z-htj2k/OpenJPH] encode error: %s\n", e.what());
    return -1;
  } catch (...) {
    fprintf(stderr, "[h5z-htj2k/OpenJPH] encode unknown error\n");
    return -1;
  }
}

extern "C" int h5z_htj2k_backend_decompress(unsigned int dtype, int num_threads,
                                            size_t compressed_nbytes,
                                            void *compressed_buffer,
                                            size_t *output_nbytes,
                                            void **output_buffer,
                                            unsigned int *output_dtype) {
  (void)num_threads; /* OpenJPH's core codec is single-threaded */

  if (!compressed_buffer || compressed_nbytes == 0 || !output_nbytes ||
      !output_buffer || !output_dtype) {
    return -1;
  }
  *output_nbytes = 0;
  *output_buffer = nullptr;
  *output_dtype = H5Z_HTJ2K_DTYPE_NONE;

  void *out = nullptr;
  try {
    ojph::mem_infile file;
    file.open(static_cast<const ojph::ui8 *>(compressed_buffer),
              compressed_nbytes);

    ojph::codestream codestream;
    codestream.read_headers(&file);

    ojph::param_siz siz = codestream.access_siz();
    ojph::ui32 ncomps = siz.get_num_components();
    if (ncomps == 0) {
      fprintf(stderr, "[h5z-htj2k/OpenJPH] no components in codestream\n");
      return -1;
    }

    ojph::ui32 width = siz.get_recon_width(0);
    ojph::ui32 height = siz.get_recon_height(0);
    ojph::ui32 precision = siz.get_bit_depth(0);
    bool is_signed = siz.is_signed(0);
    for (ojph::ui32 c = 1; c < ncomps; c++) {
      if (siz.get_recon_width(c) != width ||
          siz.get_recon_height(c) != height ||
          siz.get_bit_depth(c) != precision || siz.is_signed(c) != is_signed) {
        fprintf(stderr, "[h5z-htj2k/OpenJPH] component mismatch at c=%u\n", c);
        return -1;
      }
    }

    unsigned int out_dtype;
    size_t sample_size;
    if (dtype != H5Z_HTJ2K_DTYPE_NONE) {
      DtypeInfo di;
      if (!dtype_info(dtype, di)) {
        fprintf(stderr, "[h5z-htj2k/OpenJPH] unsupported dtype\n");
        return -1;
      }
      if (di.precision < precision) {
        fprintf(stderr,
                "[h5z-htj2k/OpenJPH] dtype precision %u too small for "
                "codestream precision %u\n",
                di.precision, precision);
        return -1;
      }
      out_dtype = dtype;
      sample_size = di.size;
    } else if (!resolve_output_dtype(precision, is_signed, &out_dtype,
                                     &sample_size)) {
      fprintf(stderr, "[h5z-htj2k/OpenJPH] unsupported precision %u\n",
              precision);
      return -1;
    }

    codestream.set_planar(false);
    codestream.create();

    const size_t npixels = static_cast<size_t>(width) * height;
    const size_t nsamples = npixels * ncomps;
    const size_t total_bytes = nsamples * sample_size;

    out = std::malloc(total_bytes);
    if (!out) {
      return -1;
    }

    for (ojph::ui32 y = 0; y < height; y++) {
      for (ojph::ui32 c = 0; c < ncomps; c++) {
        ojph::ui32 comp_num;
        const ojph::line_buf *line = codestream.pull(comp_num);
        const ojph::si32 *src = line->i32;
        const size_t row_offset = (static_cast<size_t>(y) * width) * ncomps + c;

        switch (out_dtype) {
        case H5Z_HTJ2K_DTYPE_INT8: {
          int8_t *dst = static_cast<int8_t *>(out) + row_offset;
          for (ojph::ui32 x = 0; x < width; x++)
            dst[static_cast<size_t>(x) * ncomps] = static_cast<int8_t>(src[x]);
          break;
        }
        case H5Z_HTJ2K_DTYPE_UINT8: {
          uint8_t *dst = static_cast<uint8_t *>(out) + row_offset;
          for (ojph::ui32 x = 0; x < width; x++)
            dst[static_cast<size_t>(x) * ncomps] = static_cast<uint8_t>(src[x]);
          break;
        }
        case H5Z_HTJ2K_DTYPE_INT16: {
          int16_t *dst = static_cast<int16_t *>(out) + row_offset;
          for (ojph::ui32 x = 0; x < width; x++)
            dst[static_cast<size_t>(x) * ncomps] = static_cast<int16_t>(src[x]);
          break;
        }
        case H5Z_HTJ2K_DTYPE_UINT16: {
          uint16_t *dst = static_cast<uint16_t *>(out) + row_offset;
          for (ojph::ui32 x = 0; x < width; x++)
            dst[static_cast<size_t>(x) * ncomps] =
                static_cast<uint16_t>(src[x]);
          break;
        }
        }
      }
    }

    codestream.close();

    *output_buffer = out;
    *output_nbytes = total_bytes;
    *output_dtype = out_dtype;
    return 0;
  } catch (const std::exception &e) {
    if (out) {
      std::free(out);
    }
    fprintf(stderr, "[h5z-htj2k/OpenJPH] decode error: %s\n", e.what());
    return -1;
  } catch (...) {
    if (out) {
      std::free(out);
    }
    fprintf(stderr, "[h5z-htj2k/OpenJPH] decode unknown error\n");
    return -1;
  }
}
