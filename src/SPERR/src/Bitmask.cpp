#include "Bitmask.h"

#include <algorithm>
#include <limits>

sperr::Bitmask::Bitmask(size_t nbits)
{
  if (nbits > 0) {
    auto num_longs = nbits / 64;
    if (nbits % 64 != 0)
      num_longs++;
    m_buf.assign(num_longs, 0);
    m_num_bits = nbits;
  }
}

auto sperr::Bitmask::size() const -> size_t
{
  return m_num_bits;
}

void sperr::Bitmask::resize(size_t nbits)
{
  auto num_longs = nbits / 64;
  if (nbits % 64 != 0)
    num_longs++;
  m_buf.resize(num_longs, 0);
  m_num_bits = nbits;
}

auto sperr::Bitmask::rlong(size_t idx) const -> uint64_t
{
  return m_buf[idx / 64];
}

auto sperr::Bitmask::rbit(size_t idx) const -> bool
{
  auto word = m_buf[idx / 64];
  word &= uint64_t{1} << (idx % 64);
  return (word != 0);
}

auto sperr::Bitmask::count_true() const -> size_t
{
  size_t counter = 0;
  if (m_buf.empty())
    return counter;

  // Note that unused bits in the last long are not guaranteed to be all 0's.
  for (size_t i = 0; i < m_buf.size() - 1; i++) {
    const auto val = m_buf[i];
    if (val != 0) {
      for (size_t j = 0; j < 64; j++)
        counter += ((val >> j) & uint64_t{1});
    }
  }
  const auto val = m_buf.back();
  if (val != 0) {
    for (size_t j = 0; j < m_num_bits - (m_buf.size() - 1) * 64; j++)
      counter += ((val >> j) & uint64_t{1});
  }

  return counter;
}

void sperr::Bitmask::wlong(size_t idx, uint64_t value)
{
  m_buf[idx / 64] = value;
}

void sperr::Bitmask::wbit(size_t idx, bool bit)
{
  const auto wstart = idx / 64;
  const auto mask = uint64_t{1} << (idx % 64);

  auto word = m_buf[wstart];
  if (bit)
    word |= mask;
  else
    word &= ~mask;
  m_buf[wstart] = word;
}

void sperr::Bitmask::wtrue(size_t idx)
{
  const auto wstart = idx / 64;
  const auto mask = uint64_t{1} << (idx % 64);

  auto word = m_buf[wstart];
  word |= mask;
  m_buf[wstart] = word;
}

void sperr::Bitmask::wfalse(size_t idx)
{
  const auto wstart = idx / 64;
  const auto mask = uint64_t{1} << (idx % 64);

  auto word = m_buf[wstart];
  word &= ~mask;
  m_buf[wstart] = word;
}

void sperr::Bitmask::reset()
{
  std::fill(m_buf.begin(), m_buf.end(), 0);
}

void sperr::Bitmask::reset_true()
{
  std::fill(m_buf.begin(), m_buf.end(), std::numeric_limits<uint64_t>::max());
}

auto sperr::Bitmask::view_buffer() const -> const std::vector<uint64_t>&
{
  return m_buf;
}

void sperr::Bitmask::use_bitstream(const void* p)
{
  const auto* pu64 = static_cast<const uint64_t*>(p);
  std::copy(pu64, pu64 + m_buf.size(), m_buf.begin());
}

#if defined __cpp_lib_three_way_comparison && defined __cpp_impl_three_way_comparison
auto sperr::Bitmask::operator<=>(const Bitmask& rhs) const noexcept
{
  auto cmp = m_num_bits <=> rhs.m_num_bits;
  if (cmp != 0)
    return cmp;

  if (m_num_bits % 64 == 0)
    return m_buf <=> rhs.m_buf;
  else {
    // Compare each fully used long.
    for (size_t i = 0; i < m_buf.size() - 1; i++) {
      cmp = m_buf[i] <=> rhs.m_buf[i];
      if (cmp != 0)
        return cmp;
    }
    // Compare the last partially used long.
    auto mylast = m_buf.back();
    auto rhslast = rhs.m_buf.back();
    for (size_t i = m_num_bits % 64; i < 64; i++) {
      auto mask = uint64_t{1} << i;
      mylast &= ~mask;
      rhslast &= ~mask;
    }
    return mylast <=> rhslast;
  }
}
auto sperr::Bitmask::operator==(const Bitmask& rhs) const noexcept -> bool
{
  return (operator<=>(rhs) == 0);
}
#endif
