#include "compactor.h"
#include "icecream.h"
#include <assert.h>
#include <string.h> /* memcpy() */

#ifndef NDEBUG
#include <stdio.h>
#endif

/*
 * Change this typedef to use a different width.
 * Though only uint32_t is tested so far.
 */
typedef uint32_t INT;

int compactor_strategy(const void* buf, size_t bytes)
{
  assert(bytes % 8 == 0);
  const INT all0 = 0;
  const INT all1 = ~all0;
  const INT* p = (const INT*)buf;

  size_t n0 = 0, n1 = 0;
  for (size_t i = 0; i < bytes / sizeof(INT); i++) {
    INT v = p[i];
    n0 += (v == all0);
    n1 += (v == all1);
  }

  return n1 > n0;
}

size_t compactor_comp_size(const void* buf, size_t bytes)
{
  /* The compacted bitstream has the following format:
   * -- 32 bits indicating the total number of useful bits
   * -- a single bit indicating the compaction strategy;
   * -- a bitstream encoding every INT;
   */
  assert(bytes % 8 == 0);

  const INT all0 = 0;
  const INT all1 = ~all0;
  const INT* p = (const INT*)buf;

  size_t n0 = 0, n1 = 0;
  for (size_t i = 0; i < bytes / sizeof(INT); i++) {
    INT v = p[i];
    n0 += (v == all0);
    n1 += (v == all1);
  }
  /* INTs need to be encoded verbosely */
  size_t nverb = bytes / sizeof(INT) - n0 - n1;

  size_t nbits = 33;
  if (n0 >= n1) {
    nbits += n0;
    nbits += n1 * 2;
  }
  else {
    nbits += n1;
    nbits += n0 * 2;
  }
  nbits += nverb * (2 + 8 * sizeof(INT));

  size_t nbytes = (nbits + 7) / 8;
  return nbytes;
}

size_t compactor_useful_bytes(const void* comp_buf)
{
  uint32_t nbits = 0;
  memcpy(&nbits, comp_buf, sizeof(nbits));

  return (nbits + 7) / 8;
}

size_t compactor_encode(const void* bitmask,
                        size_t bitmask_bytes,
                        void* compact_bitstream,
                        size_t compact_bitstream_bytes)
{
  assert(bitmask_bytes % 8 == 0);
  assert(compact_bitstream_bytes % 8 == 0);

  /* decide on the compaction strategy */
  INT most_freq = 0;
  INT next_freq = ~most_freq;
  int strategy = compactor_strategy(bitmask, bitmask_bytes);
  if (strategy) {
    next_freq = 0;
    most_freq = ~next_freq;
  }

  icecream out;
  icecream_use_mem(&out, compact_bitstream, compact_bitstream_bytes);

  /* skip 32 bits for total bit count storage, and then keep the strategy. */
  for (int i = 0; i < 32; i++)
    icecream_wbit(&out, 0);
  icecream_wbit(&out, strategy);

  /* encode the bitmask, one INT at a time */
  const INT* p = (const INT*)bitmask;
  for (size_t i = 0; i < bitmask_bytes / sizeof(INT); i++) {
    INT v = p[i];
    if (v == most_freq)
      icecream_wbit(&out, 0);
    else if (v == next_freq) {
      icecream_wbit(&out, 1);
      icecream_wbit(&out, 0);
    }
    else {
      icecream_wbit(&out, 1);
      icecream_wbit(&out, 1);
      for (int j = 0; j < 8 * sizeof(INT); j++) {
        int bit = (v >> j) & (INT)1;
        icecream_wbit(&out, bit);
      }
    }
  }

  size_t nbits = icecream_wtell(&out);
  icecream_flush(&out);

  /* make sure that the total number of bits can fit in a 32-bit integer,
   * and keep it at the beginning of the output bitstream. */
  uint32_t tmp = 0;
  assert(nbits <= ~tmp);
  tmp = (uint32_t)nbits;
  memcpy(compact_bitstream, &tmp, sizeof(tmp));

  return (nbits + 7) / 8;
}

size_t compactor_decode(const void* compact_bitstream,
                        size_t compact_bitstream_bytes,
                        void* decoded_bitmask)
{
  assert(compact_bitstream_bytes % 8 == 0);

  /* wrap the `compact_bitstream` in an icecream. As long as we don't write,
   * its content won't be changed. */ 
  icecream in;
  icecream_use_mem(&in, (void*)compact_bitstream, compact_bitstream_bytes);

  /* extract the total number of useful bits, then skip the first 32 bits. */
  uint32_t nbits = 0;
  memcpy(&nbits, compact_bitstream, sizeof(nbits));
  for (int i = 0; i < 32; i++)
    icecream_rbit(&in);

  /* decide on the compaction strategy. */
  INT most_freq = 0;
  INT next_freq = ~most_freq;
  int strategy = icecream_rbit(&in);
  if (strategy) {
    next_freq = 0;
    most_freq = ~next_freq;
  }

  /* decode the bitmask one INT at a time */
  INT* p = (INT*)decoded_bitmask;
  while (icecream_rtell(&in) < nbits) {
    int bit = icecream_rbit(&in);
    if (bit == 0)   /* produce a most frequent INT */
      *p++ = most_freq;
    else {
      assert(icecream_rtell(&in) < nbits);
      bit = icecream_rbit(&in);
      if (bit == 0) /* produce a second most frequent INT */
        *p++ = next_freq;
      else {        /* read the next INT verbosely */
        INT v = 0;
        for (int j = 0; j < 8 * sizeof(INT); j++) {
          assert(icecream_rtell(&in) < nbits);
          bit = icecream_rbit(&in);
          v |= (INT)bit << j;
        }
        *p++ = v;
      }
    }
  }

  return (p - (INT*)decoded_bitmask) * sizeof(INT);
}
