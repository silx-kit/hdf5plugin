//
//  Operators.hpp
//  Operators
//
//  Created by Jan Pieter Abrahams on 13/04/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef Operators_h
#define Operators_h
#include <complex>

// Some useful operators and constants in a namespace Operator:: that are not defined in the C++ standard:
//
//  static const bool big_endian
//      Runtime function returning true if running on a big-endian machine, otherwise returns false. Will
//      be superceded in C++20 by the class std::endian.
//  swap_bytes(T val)
//      Swaps byte order of 'val'. Note that 'val' can also be of type std::complex<T>
//  euclidian_remainder(T v, Ti p)
//      Returns the Euclidian remainer (positive modulus). Faster than operator%() or fmod() when 'v' is
//      within the interval [-p; 2p] and 'p' is positive. When 'v' outside this interval, or when (p<0),
//      use ((v %= p) < 0) ? v+abs(p) : v for integral types, or ((v = fmod(v,p) < 0) ? v+abs(p) : v for
//      real-valued types.
//  euclidian_remainder<int N>(T v)
//      Returns the Euclidian remainer for N is a positive power of 2 and 'v' is an integral type.
//  euclidian_division(T v, Ti p)
//      Returns an anonymous struct with members (anonymous)::quot and (anonymous)::rem, which
//      are the integral divisor and the positive remainder of v / p. The type of the quotient and
//      remainder is decltype(T*Ti).
//  euclidian_division<std::uint32_t N>(T v)
//      Fast euclidian division for N is a positive power of 2 and 'v' is an integral type. If N is not
//      a positive power of 2, euclidian_division(v, N) is called instead.
//  int highest_set_bit(T val)
//      Returns the number of the most significant bit that is set in an integral type T. If no bits are
//      set, 0 is returned. If T is signed, the sign bit is added, unless (val == 0). So
//      (highest_set_bit(uint8_t(0b00100000)) == 6), whilst for a signed argument:
//      (highest_set_bit( int8_t(0b00100000)) == 7).
//  int lowest_set_bit(T val)
//      Returns the number of the least significant bit that is set in an integral type T. If no bits
//      are set, returns 0. So (lowest_set_bit(uint8_t(0b00100100)) == 3) and (lowest_set_bit(0) == 0).
//  bool is_bounded(std::initializer_list<T> const& list)
//      Returns true if the values of 'list' between the first and last item on the list, are at least
//      equal to the first item of the list and smaller than the last item of the list.
//  std::string filename_extension(std::string const& filename)
//      Returns the filename extension: string starting at the last occurrence of the character '.'
//      For instance: filename_extension("foo.bar") == ".bar".
//  std::string strip_filename_extension(std::string const& filename)
//      Returns the filename without extension: string starting up to the last occurrence of the character '.'
//      For instance: filename_extension("foo.bar") == "foo".

namespace Operator {

static const bool big_endian = []() {
    int i=1;
    return ! *(reinterpret_cast<uint8_t*>(&i));
}();

template <typename T>
constexpr inline T swap_bytes(T val) noexcept {
    uint8_t *s = reinterpret_cast<uint8_t*>(&val);
    uint8_t *e = s + sizeof(T) - 1;
    while (s < e) std::swap(*s++, *e--);
    return val;
}

template <typename T>
constexpr inline std::complex<T> swap_bytes(std::complex<T> const& val) noexcept {
    T r = val.real();
    T i = val.imag();
    return std::complex<T>(swap_bytes(r), swap_bytes(i));
}

template <typename T0, typename T1 = T0>
constexpr inline decltype(T0()*T1()) euclidian_remainder(T0 v, T1 const p) noexcept {
    if constexpr (std::is_signed_v<T0>) {
        if (((v <  0) && ((v += p) <  0)) ||
            ((v >= p) && ((v -= p) >= p))) {
            if constexpr (std::is_integral_v<decltype(v*p)>) {
                if constexpr (std::is_signed_v<T1>)
                    return ((v -= (v / p) * p) < 0) ? v+abs(p) : v; // not use % as this may have a similar optimisation
                else if (v < 0)
                    return p - ((-v) - ((-v) / p) * p);
                else
                    return v - (v / p) * p;
            }
            else {
                if constexpr (std::is_signed_v<T1>)
                    return ((v = fmod(v,p)) < 0) ? v+abs(p) : v;
                else
                    return ((v = fmod(v,p)) < 0) ? v+p : v;
            }
        }
    }
    else if constexpr (std::is_signed_v<T1>) {
        if ((p > 0) && (v >= p) && ((v -= p) >= p)) {
            if constexpr (std::is_integral_v<decltype(v*p)>)
                return v - (v / p) * p;
            else
                return v = fmod(v,p);
        }
        else if ((v += p) >= (-p)) {
            if constexpr (std::is_integral_v<decltype(v*p)>)
                return v - (v / (-p)) * (-p);
            else
                return v = fmod(v,p);
        }
    }
    else if ((v >= p) && ((v -= p) >= p))
        return v - (v / p) * p;
    return v;
}

template <std::uint16_t const N, typename T0>
constexpr inline T0 euclidian_remainder(T0 const v) noexcept {
    if constexpr (std::is_integral_v<T0> && ((N & (N - 1)) == 0))
        return v & N-1;
    else
        return euclidian_remainder(v, N);
}

template <typename T0, typename T1 = T0>
constexpr inline auto euclidian_division(T0 const v, T1 const p) noexcept {
    if constexpr (std::is_integral_v<decltype(v*p)>) {
        auto r = std::div( v,  p);
        if (r.rem < 0) {
            r.rem += p;
            --r.quot;
        }
        return r;
    }
    else {
        struct {decltype(T0()*T1()) quot; decltype(T0()*T1()) rem;} r {};
        r.rem = euclidian_remainder(v, p);
        r.quot = (v - r.rem) / p;
        return r;
    }
}

template <std::uint16_t const N, typename T0>
constexpr inline auto euclidian_division(T0 const v) noexcept {
    if constexpr (std::is_integral_v<T0> && ((N & (N - 1)) == 0)) {
        struct {T0 quot; T0 rem;} r {};
        r.rem = v & T0(N-1);
        r.quot = (v - r.rem) / T0(N);
        return r;
    }
    else
        return euclidian_division(v, N);
}

template <typename T>
constexpr inline int highest_set_bit(T val) noexcept {
    static_assert(std::is_integral_v<T> , "most_significant_bit() requires an integral argument.");
    if constexpr (std::is_signed_v<T>)
        return (val == 0) ? 0 : 1 + highest_set_bit(std::make_unsigned_t<T> (abs(val)));
    else {
        int r=0;
        for ( ; val; val>>=1, ++r);
        return r;
    }
}

template <typename T>
constexpr inline int lowest_set_bit(T val) noexcept {
    static_assert(std::is_integral_v<T> , "least_significant_bit() requires an integral argument.");
    if (val) {
        int r=1;
        val = (val ^ (val - 1)) >> 1;
        for ( ; val; val>>=1, ++r);
        return r;
    }
    else return 0;
}

template <typename T>
constexpr inline bool is_bounded(std::initializer_list<T> const& list) noexcept {
    auto const last = list.end() - 1;
    for (auto p = list.begin() + 1;  p != last; ++p)
        if ((*p < *list.begin()) || (*p >= *last))
            return false;
    return true;
}

inline std::string filename_extension(std::string const& filename) noexcept {
    return filename.substr(filename.find_last_of('.'));
}

    inline std::string strip_filename_extension(std::string const& filename) noexcept {
        return filename.substr(0, filename.find_last_of('.'));
    }

}
#endif /* Operators_h */
