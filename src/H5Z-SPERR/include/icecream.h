/*
 * This is a mimic of the Bitstream class in SPERR:
 * https://github.com/NCAR/SPERR/blob/main/include/Bitstream.h
 *
 * The most significant difference is that bitstream here doesn't manage
 * any memory; it reads a bit sequence from a user-provided memory buffer,
 * or writes a bit sequence to a user-provided memory buffer.
 * 
 * The "object" is named `icecream` and all functions operating on it
 * are named with a prefix `icecream`.
 */

#ifndef ICECREAM_H
#define ICECREAM_H

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#ifndef NDEBUG
#include <stdio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  uint64_t* begin;  /* begin of the stream */
  uint64_t* ptr;    /* pointer to the next word to be read/written */
  uint64_t  buffer; /* incoming/outgoing bits */
  int       bits;   /* number of buffered bits (0 <= bits < 64) */
} icecream;

/*
 * Specify a bitstream to use memory provided by users.
 * NOTE: the memory length (in bytes) have to be a multiplier of 8,
 * because the icecream class writes/reads in 64-bit integers.
 */
void icecream_use_mem(icecream* s, void* mem, size_t bytes);

/* Position the bitstream for reading or writing at the beginning. */
void icecream_rewind(icecream* s);

/* Read a bit. Please don't read beyond the end of the stream. */
int icecream_rbit(icecream* s);

/* Write a bit (0 or 1). Please don't write beyond the end of the stream. */
void icecream_wbit(icecream* s, int bit);

/* Return the bit offset to the next bit to be read. */
size_t icecream_rtell(icecream* s);

/* Return the bit offset to the next bit to be written. */
size_t icecream_wtell(icecream* s);

/* Write any remaining buffered bits and align stream on next word boundary. */
void icecream_flush(icecream* s);

#ifdef __cplusplus
}
#endif

#endif
