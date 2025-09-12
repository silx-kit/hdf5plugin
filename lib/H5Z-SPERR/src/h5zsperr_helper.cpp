#include <algorithm>
#include <cassert>
#include <cmath>  // isnan()
#include <memory>

#include <H5PLextern.h>
#include <hdf5.h>
#include "h5zsperr_helper.h"

#include "compactor.h"
#include "icecream.h"

unsigned int C_API::h5zsperr_pack_extra_info(int rank, int is_float, int missing_val_mode, int magic)
{
  assert(rank == 3 || rank == 2);
  assert(is_float == 1 || is_float == 0);
  assert(missing_val_mode >= 0 && missing_val_mode <= 2);
  assert(magic >= 0 && magic <= 63);

  unsigned int ret = 0;

  // Bit positions 0-3 to encode the rank.
  if (rank == 2)
    ret |= 1u << 1; // Position 1
  else {
    ret |= 1u;      // Position 0
    ret |= 1u << 1; // Position 1
  }

  // Bit positions 4-5 encode data type.
  if (is_float == 1)
    ret |= 1u << 4; // Position 4

  // Bit positions 6-9 encode missing value mode.
  ret |= missing_val_mode << 6;

  // Bit positions 10-15 encode the magic number.
  ret |= magic << 10;

  return ret;
}

void C_API::h5zsperr_unpack_extra_info(unsigned int meta,
                                       int* rank,
                                       int* is_float,
                                       int* missing_val_mode,
                                       int* magic)
{
  // Extract rank from bit positions 0-3.
  unsigned pos0 = meta & 1u;
  unsigned pos1 = meta & (1u << 1);
  if (!pos0 && pos1)
    *rank = 2;
  else if (pos0 && pos1)
    *rank = 3;
  else { // error
    H5Epush(H5E_DEFAULT, __FILE__, __func__, __LINE__, H5E_ERR_CLS, H5E_PLINE, H5E_BADVALUE,
            "Rank is not 2 or 3.");
  }

  // Extract data type from positions 4-5.
  unsigned pos4 = meta & (1u << 4);
  if (pos4)
    *is_float = 1;
  else
    *is_float = 0; // is_double

  // Extract missing value mode from positions 6-9.
  *missing_val_mode = 15; // 2^4 = 16
  *missing_val_mode &= meta >> 6;

  // Extract the magic number from positions 10-15.
  *magic = 63;  // 2^6 = 64
  *magic &= meta >> 10;
}

int C_API::h5zsperr_has_nan(const void* buf, size_t nelem, int is_float)
{
  assert(is_float == 1 || is_float == 0);
  if (is_float) {
    const float* p = (const float*)buf;
    return std::any_of(p, p + nelem, [](auto v) { return std::isnan(v); });
  }
  else {
    const double* p = (const double*)buf;
    return std::any_of(p, p + nelem, [](auto v) { return std::isnan(v); });
  }
}

int C_API::h5zsperr_has_large_mag(const void* buf, size_t nelem, int is_float)
{
  assert(is_float == 1 || is_float == 0);
  if (is_float) {
    const float* p = (const float*)buf;
    return std::any_of(p, p + nelem, [](auto v) { return std::abs(v) >= LARGE_MAGNITUDE_F; });
  }
  else {
    const double* p = (const double*)buf;
    return std::any_of(p, p + nelem, [](auto v) { return std::abs(v) >= LARGE_MAGNITUDE_D; });
  }
}

int C_API::h5zsperr_make_mask_nan(const void* data_buf, size_t nelem, int is_float,
                                  void* mask_buf, size_t mask_bytes, size_t* useful_bytes)
{
  assert(is_float == 0 || is_float == 1);

  // First, make a naive mask.
  //
  auto nbytes = (nelem + 7) / 8;
  while (nbytes % 8)
    nbytes++;
  auto mem = std::make_unique<uint8_t[]>(nbytes);
  auto s1 = icecream();
  icecream_use_mem(&s1, mem.get(), nbytes);

  if (is_float) {
    const float* p = (const float*)data_buf;
    for (size_t i = 0; i < nelem; i++)
      icecream_wbit(&s1, std::isnan(p[i]));
  }
  else {
    const double* p = (const double*)data_buf;
    for (size_t i = 0; i < nelem; i++)
      icecream_wbit(&s1, std::isnan(p[i]));
  }
  icecream_flush(&s1);

  // Second, compact this naive mask.
  //
  while (mask_bytes % 8)
    mask_bytes--;
  if (mask_bytes < compactor_comp_size(mem.get(), nbytes))
    return 1; // Not enough space!

  *useful_bytes = compactor_encode(mem.get(), nbytes, mask_buf, mask_bytes);

  return 0;
}

int C_API::h5zsperr_make_mask_large_mag(const void* data_buf, size_t nelem, int is_float,
                                        void* mask_buf, size_t mask_bytes, size_t* useful_bytes)
{
  assert(is_float == 0 || is_float == 1);

  // First, make a naive mask.
  //
  auto nbytes = (nelem + 7) / 8;
  while (nbytes % 8)
    nbytes++;
  auto mem = std::make_unique<uint8_t[]>(nbytes);
  auto s1 = icecream();
  icecream_use_mem(&s1, mem.get(), nbytes);

  if (is_float) {
    const float* p = (const float*)data_buf;
    for (size_t i = 0; i < nelem; i++)
      icecream_wbit(&s1, std::abs(p[i]) >= LARGE_MAGNITUDE_F);
  }
  else {
    const double* p = (const double*)data_buf;
    for (size_t i = 0; i < nelem; i++)
      icecream_wbit(&s1, std::abs(p[i]) >= LARGE_MAGNITUDE_D);
  }
  icecream_flush(&s1);

  // Second, compact this naive mask.
  //
  while (mask_bytes % 8)
    mask_bytes--;
  if (mask_bytes < compactor_comp_size(mem.get(), nbytes))
    return 1; // Not enough space!

  *useful_bytes = compactor_encode(mem.get(), nbytes, mask_buf, mask_bytes);

  return 0;
}

template<typename T>
T treat_nan_impl(T* buf, size_t nelem)
{
  // First, find the mean value.
  const size_t BLOCK = 2048;
  double total_sum = 0.0, block_sum = 0.0;
  size_t total_cnt = 0, block_cnt = 0;

  for (size_t i = 0; i < nelem; i++) {
    if (!std::isnan(buf[i])) {
      block_sum += double(buf[i]);
      if (++block_cnt == BLOCK) {
        total_sum += block_sum;
        total_cnt += BLOCK;
        block_sum = 0.0;
        block_cnt = 0;
      }
    }
  }
  double mean = (total_sum + block_sum) / double(total_cnt + block_cnt);

  // Second, replace every occurance of NaN
  std::replace_if(buf, buf + nelem, [](auto v) { return std::isnan(v); }, T(mean));

  return T(mean);
}
float C_API::h5zsperr_treat_nan_f32(float* data_buf, size_t nelem)
{
  return treat_nan_impl(data_buf, nelem);
}
double C_API::h5zsperr_treat_nan_f64(double* data_buf, size_t nelem)
{
  return treat_nan_impl(data_buf, nelem);
}

template<typename T>
T treat_large_mag_impl(T* buf, size_t nelem)
{
  // First, find the mean value.
  constexpr T MAG = sizeof(T) == 4 ? LARGE_MAGNITUDE_F : LARGE_MAGNITUDE_D;
  const size_t BLOCK = 2048;
  double total_sum = 0.0, block_sum = 0.0;
  size_t total_cnt = 0, block_cnt = 0;

  for (size_t i = 0; i < nelem; i++) {
    if (std::abs(buf[i]) < MAG) {
      block_sum += double(buf[i]);
      if (++block_cnt == BLOCK) {
        total_sum += block_sum;
        total_cnt += BLOCK;
        block_sum = 0.0;
        block_cnt = 0;
      }
    }
  }
  double mean = (total_sum + block_sum) / double(total_cnt + block_cnt);

  // Second, find the first large magnitude value.
  auto orig = *(std::find_if(buf, buf + nelem, [MAG](auto v) { return std::abs(v) >= MAG; }));

  // Third, replace every occurance of large magnitude
  std::replace_if(buf, buf + nelem, [MAG](auto v) { return std::abs(v) >= MAG; }, T(mean));

  return orig;
}
float C_API::h5zsperr_treat_large_mag_f32(float* data_buf, size_t nelem)
{
  return treat_large_mag_impl(data_buf, nelem);
}
double C_API::h5zsperr_treat_large_mag_f64(double* data_buf, size_t nelem)
{
  return treat_large_mag_impl(data_buf, nelem);
}
