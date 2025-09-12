#include "Bitstream.h"

#include <cassert>
#include <cstring>
#include <iterator>  // std::distance()

// Constructor
sperr::Bitstream::Bitstream(size_t nbits)
{
  m_itr = m_buf.begin();
  this->reserve(nbits);
}

// Functions for both read and write
void sperr::Bitstream::rewind()
{
  m_itr = m_buf.begin();
  m_buffer = 0;
  m_bits = 0;
}

auto sperr::Bitstream::capacity() const -> size_t
{
  return m_buf.size() * 64;
}

void sperr::Bitstream::reserve(size_t nbits)
{
  if (nbits > m_buf.size() * 64) {
    // Number of longs that's absolutely needed.
    auto num_longs = nbits / 64;
    if (num_longs * 64 < nbits)
      num_longs++;

    const auto dist = std::distance(m_buf.begin(), m_itr);
    m_buf.resize(num_longs);         // trigger a memroy allocation.
    m_buf.resize(m_buf.capacity());  // be able to make use of all available capacity.
    m_itr = m_buf.begin() + dist;
  }
}

void sperr::Bitstream::reset()
{
  std::fill(m_buf.begin(), m_buf.end(), 0);
}

// Functions for read
auto sperr::Bitstream::rtell() const -> size_t
{
  // Stupid C++ insists that `m_buf.begin()` gives me a const iterator...
  std::vector<uint64_t>::const_iterator itr2 = m_itr;  // NOLINT
  return std::distance(m_buf.begin(), itr2) * 64 - m_bits;
}

void sperr::Bitstream::rseek(size_t offset)
{
  size_t div = offset / 64;
  size_t rem = offset - div * 64;
  m_itr = m_buf.begin() + div;
  if (rem) {
    m_buffer = *m_itr >> rem;
    ++m_itr;
    m_bits = 64 - rem;
  }
  else {
    m_buffer = 0;
    m_bits = 0;
  }
}

auto sperr::Bitstream::rbit() -> bool
{
  if (m_bits == 0) {
    m_buffer = *m_itr;
    ++m_itr;
    m_bits = 64;
  }
  --m_bits;
  bool bit = m_buffer & uint64_t{1};
  m_buffer >>= 1;
  return bit;
}

// Functions for write
auto sperr::Bitstream::wtell() const -> size_t
{
  // Stupid C++ insists that `m_buf.begin()` gives me a const iterator...
  std::vector<uint64_t>::const_iterator itr2 = m_itr;  // NOLINT
  return std::distance(m_buf.begin(), itr2) * 64 + m_bits;
}

void sperr::Bitstream::wseek(size_t offset)
{
  size_t div = offset / 64;
  size_t rem = offset - div * 64;
  m_itr = m_buf.begin() + div;
  if (rem) {
    m_buffer = *m_itr;
    m_buffer &= (uint64_t{1} << rem) - 1;
    m_bits = rem;
  }
  else {
    m_buffer = 0;
    m_bits = 0;
  }
}

void sperr::Bitstream::wbit(bool bit)
{
  m_buffer |= uint64_t{bit} << m_bits;

#if __has_cpp_attribute(unlikely)
  if (++m_bits == 64) [[unlikely]]
#else
  if (++m_bits == 64)
#endif
  {
    if (m_itr == m_buf.end()) {  // allocate memory if necessary.
      auto dist = m_buf.size();
      m_buf.resize(std::max(size_t{1}, dist) * 2 - dist / 2);  // use a growth factor of 1.5
      m_itr = m_buf.begin() + dist;
    }
    *m_itr = m_buffer;
    ++m_itr;
    m_buffer = 0;
    m_bits = 0;
  }
}

void sperr::Bitstream::flush()
{
  if (m_bits) {  // only really flush when there are remaining bits.
    if (m_itr == m_buf.end()) {
      auto dist = m_buf.size();
      m_buf.resize(std::max(size_t{1}, dist) * 2 - dist / 2);  // use a growth factor of 1.5
      m_itr = m_buf.begin() + dist;
    }
    *m_itr = m_buffer;
    ++m_itr;
    m_buffer = 0;
    m_bits = 0;
  }
}

// Functions to provide or parse a compact bitstream
void sperr::Bitstream::write_bitstream(void* p, size_t num_bits) const
{
  assert(num_bits <= m_buf.size() * 64);

  const auto num_longs = num_bits / 64;
  auto rem_bytes = num_bits / 8 - num_longs * sizeof(uint64_t);
  if (num_bits % 8 != 0)
    rem_bytes++;

  if (num_longs > 0)
    std::memcpy(p, m_buf.data(), num_longs * sizeof(uint64_t));

  if (rem_bytes > 0) {
    uint64_t value = m_buf[num_longs];
    auto* const p_byte = static_cast<std::byte*>(p);
    std::memcpy(p_byte + num_longs * sizeof(uint64_t), &value, rem_bytes);
  }
}

auto sperr::Bitstream::get_bitstream(size_t num_bits) const -> std::vector<std::byte>
{
  assert(num_bits <= m_buf.size() * 64);

  auto num_bytes = num_bits / 8;
  if (num_bits - num_bytes * 8 != 0)
    num_bytes++;

  auto tmp = std::vector<std::byte>(num_bytes);
  this->write_bitstream(tmp.data(), num_bits);

  return tmp;
}

void sperr::Bitstream::parse_bitstream(const void* p, size_t num_bits)
{
  this->reserve(num_bits);

  const auto num_longs = num_bits / 64;
  auto rem_bytes = num_bits / 8 - num_longs * sizeof(uint64_t);
  if (num_bits % 8 != 0)
    rem_bytes++;

  const auto* const p_byte = static_cast<const std::byte*>(p);

  if (num_longs > 0)
    std::memcpy(m_buf.data(), p_byte, num_longs * sizeof(uint64_t));

  if (rem_bytes > 0)
    std::memcpy(m_buf.data() + num_longs, p_byte + num_longs * sizeof(uint64_t), rem_bytes);

  this->rewind();
}
