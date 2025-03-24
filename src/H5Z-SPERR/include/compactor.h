/* This is a set of functions that compact a bitmask.
 * In the intended use case, a bitmask is produced by masking all
 * "missing values" or "fill values" in a model output with zero's,
 * whereas the locations with valid data points are marked with one's.
 * However, this compactor is likely to be effective with any bit patterns
 * that have lots of consecutive 0's or 1's.
 *
 * The bitmask compactor works in the following way:
 * 1. Assume that we use 32-bit ints; the compactor encodes 32 bits at a time.
 * 2. Every incoming int is encoded in one of three ways:
 *    2.1. For an int with all 0's, use a single 0 bit.
 *    2.2. For an int with all 1's, use two bits: 10.
 *    2.3. For all other ints, use 34 bits: two bits 11 then followed by the
 *         verbose presentation of the 32-bit int.
 * 3. Obviously, it's the most economical to use a single 0 bit to present
 *    the most frequent pattern (all 0's or all 1's). The encoder thus does
 *    a test at the beginning and records the test result.
 */

#ifndef COMPACTOR_H
#define COMPACTOR_H

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Return the compaction strategy to use:
 * 0: compact with all 0's being the most frequent
 * 1: compact with all 1's being the most frequent
 * Note: `bytes` has to be a multiple of 8.
 */
int compactor_strategy(const void* buf, size_t bytes);

/* Return the size in bytes of the resulting compacted bitstream, given an input buf.
 * Note: `buf_bytes` has to be a multiple of 8.
 */
size_t compactor_comp_size(const void* buf, size_t buf_bytes);

/* Return the number of useful bytes in a compacted bitstream.
 * This value is the same as the output of `compactor_comp_size()` during encoding.
 */
size_t compactor_useful_bytes(const void* comp_buf);

/* Return the useful size of the output bitstream,
 * which is the same as the output of `compactor_comp_size()`.
 * Note 1: the input bitmask length (in bytes) has to be a multiple of 8.
 *         This requirement is inheritated from the bitstream implementation.
 * Note 2: the output buffer length should be 1) a multiple of 8, and
 *         2) no less than the size returned by `compactor_comp_size()`.
 */
size_t compactor_encode(const void* bitmask,
                        size_t bitmask_bytes,
                        void* compact_bitstream,
                        size_t compact_bitstream_bytes);

/* Return the number of useful bytes in the decoded bitmask.
 * Note: The number of useful bytes might be bigger than the number of bytes being
 *       encoded, because of the word size that the compactor operates on.
 * Note: `compact_bitstream_bytes` should be a multiple of 8 that is no less than
 *       the size returned by `compactor_encode()`.
 */
size_t compactor_decode(const void* compact_bitstream,
                        size_t compact_bitstream_bytes,
                        void* decoded_bitmask);

#ifdef __cplusplus
}
#endif

#endif
