#include "icecream.h"

void icecream_use_mem(icecream* s, void* mem, size_t bytes) {
  s->begin = (uint64_t*)mem;
  icecream_rewind(s);
}

void icecream_rewind(icecream* s)
{
  s->ptr = s->begin;
  s->buffer = 0;
  s->bits = 0;
}

int icecream_rbit(icecream* s)
{
  if (!s->bits) {
    s->buffer = *(s->ptr);
    (s->ptr)++;
    s->bits = 64;
  }
  (s->bits)--;
  int bit = s->buffer & (uint64_t)1;
  s->buffer >>= 1;
  return bit;
}

void icecream_wbit(icecream* s, int bit)
{
  s->buffer |= (uint64_t)bit << s->bits;
    
  if (++(s->bits) == 64) {
    *(s->ptr) = s->buffer;
    (s->ptr)++;
    s->bits = 0;
    s->buffer = 0;
  }
}

size_t icecream_wtell(icecream* s)
{
  return (s->ptr - s->begin) * (size_t)64 + s->bits;
}

size_t icecream_rtell(icecream* s)
{
  return (s->ptr - s->begin) * (size_t)64 - s->bits;
}

void icecream_flush(icecream* s)
{
  if (s->bits) {  /* only really flush when there are remaining bits */
    *(s->ptr) = s->buffer;
    (s->ptr)++; 
    s->buffer = 0;
    s->bits = 0;
  }
}
