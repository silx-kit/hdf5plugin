#include <sys/stat.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <vector>

using FLOAT = double;

int sam_read_n_bytes(const char* filename,
                     size_t n_bytes, /* input  */
                     void* buffer)   /* output */
{
  FILE* f = fopen(filename, "r");
  if (f == NULL) {
    fprintf(stderr, "Error! Cannot open input file: %s\n", filename);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  if (ftell(f) < n_bytes) {
    fprintf(stderr, "Error! Input file size error: %s\n", filename);
    fprintf(stderr, "  Expecting %lu bytes, got %ld bytes.\n", n_bytes, ftell(f));
    fclose(f);
    return 1;
  }
  fseek(f, 0, SEEK_SET);

  if (fread(buffer, 1, n_bytes, f) != n_bytes) {
    fprintf(stderr, "Error! Input file read error: %s\n", filename);
    fclose(f);
    return 1;
  }

  fclose(f);
  return 0;
}

template <typename T>
void calc_stats(const T* arr1,
                const T* arr2,
                size_t len,
                T& rmse,
                T& linfty,
                T& psnr,
                T& arr1min,
                T& arr1max)
{
  //
  // Calculate min and max of arr1
  //
  const auto minmax = std::minmax_element(arr1, arr1 + len);
  arr1min = *minmax.first;
  arr1max = *minmax.second;

  //
  // In rare cases, the two input arrays are identical.
  //
  auto mism = std::mismatch(arr1, arr1 + len, arr2, arr2 + len);
  if (mism.first == arr1 + len && mism.second == arr2 + len) {
    rmse = 0.0;
    linfty = 0.0;
    psnr = std::numeric_limits<T>::infinity();
    return;
  }

  const size_t stride_size = 4096;
  const size_t num_of_strides = len / stride_size;
  const size_t remainder_size = len - stride_size * num_of_strides;

  auto sum_vec = std::vector<T>(num_of_strides + 1);
  auto linfty_vec = std::vector<T>(num_of_strides + 1);

  //
  // Calculate summation and l-infty of each stride
  //
  for (size_t stride_i = 0; stride_i < num_of_strides; stride_i++) {
    T maxerr = 0.0;
    auto buf = std::array<T, stride_size>();
    for (size_t i = 0; i < stride_size; i++) {
      const size_t idx = stride_i * stride_size + i;
      auto diff = std::abs(arr1[idx] - arr2[idx]);
      maxerr = std::max(maxerr, diff);
      buf[i] = diff * diff;
    }
    sum_vec[stride_i] = std::accumulate(buf.begin(), buf.end(), T{0.0});
    linfty_vec[stride_i] = maxerr;
  }

  //
  // Calculate summation and l-infty of the remaining elements
  //
  T last_linfty = 0.0;
  auto last_buf = std::array<T, stride_size>{};  // must be enough for
                                                 // `remainder_size` elements.
  for (size_t i = 0; i < remainder_size; i++) {
    const size_t idx = stride_size * num_of_strides + i;
    auto diff = std::abs(arr1[idx] - arr2[idx]);
    last_linfty = std::max(last_linfty, diff);
    last_buf[i] = diff * diff;
  }
  sum_vec[num_of_strides] =
      std::accumulate(last_buf.begin(), last_buf.begin() + remainder_size, T{0.0});
  linfty_vec[num_of_strides] = last_linfty;

  //
  // Now calculate linfty
  //
  linfty = *(std::max_element(linfty_vec.begin(), linfty_vec.end()));

  //
  // Now calculate rmse and psnr
  // Note: psnr is calculated in dB, and follows the equation described in:
  // http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/VELDHUIZEN/node18.html
  // Also refer to https://www.mathworks.com/help/vision/ref/psnr.html
  //
  const auto msr = std::accumulate(sum_vec.begin(), sum_vec.end(), T{0.0}) / T(len);
  rmse = std::sqrt(msr);
  auto range_sq = *minmax.first - *minmax.second;
  range_sq *= range_sq;
  psnr = std::log10(range_sq / msr) * 10.0;
}

int main(int argc, char* argv[])
{
  if (argc != 3) {
    printf("Usage: ./a.out file1 file2\n");
    return 1;
  }

  const char* file1 = argv[1];
  const char* file2 = argv[2];

  struct stat st1, st2;
  stat(file1, &st1);
  stat(file2, &st2);
  if (st1.st_size != st2.st_size) {
    printf("Two files have different sizes!\n");
    return 1;
  }

  auto n_bytes = st1.st_size;
  auto n_vals = n_bytes / sizeof(FLOAT);

  auto buf1 = std::make_unique<FLOAT[]>(n_vals);
  auto buf2 = std::make_unique<FLOAT[]>(n_vals);

  sam_read_n_bytes(file1, n_bytes, buf1.get());
  sam_read_n_bytes(file2, n_bytes, buf2.get());

  FLOAT rmse, lmax, psnr, arr1min, arr1max;
  calc_stats(buf1.get(), buf2.get(), n_vals, rmse, lmax, psnr, arr1min, arr1max);
  printf("rmse = %e, lmax = %e, psnr = %fdB, orig_min = %f, orig_max = %f\n", rmse, lmax, psnr,
         arr1min, arr1max);
}
