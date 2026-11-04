//
//  Bitqueue.h
//  Bitqueue
//
//  Created by Jan Pieter Abrahams on 14.12.2024.
//

#ifndef Bitqueue_h
#define Bitqueue_h

#include "Shared_array.hpp"
#include <cassert>
#include <cstring>
#include <bit>
#include <numeric>
#include <vector>
#include <concepts>

namespace jpa {

/**
 * @brief Concept to define a Bitqueue-like container.
 *
 * A container that:
 * - Provides sequential access to its data.
 * - Has a `data()` member returning a pointer to the underlying memory (`std::uint8_t*`).
 * - Offers `begin()` and `end()` methods for iterating over the memory sequentially.
 * - Ensures compatibility between the raw pointer returned by `data()` and the iterator returned by `begin()`.
 *
 * This concept can be used to constrain generic code to only accept containers that satisfy the
 * required properties of a Bitqueue.
 *
 * **Requirements:**
 * - `data()` must return a `std::uint8_t*`.
 * - `begin()` and `end()` must return iterators, and `*begin()` must yield a `std::uint8_t&`.
 * - The iterator returned by `begin()` must satisfy the `std::contiguous_iterator` concept.
 * - `data()` and `std::to_address(begin())` must return the same address.
 *
 * This ensures that the container stores its values sequentially in memory and allows efficient access.
 */
template <typename T>
concept Bitqueue = requires(T obj) {
    { obj.data() } -> std::same_as<std::uint8_t*>;
    { obj.begin() } -> std::forward_iterator;
    { obj.end() } -> std::forward_iterator;
    { *obj.begin() } -> std::same_as<std::uint8_t&>;
    requires std::contiguous_iterator<decltype(obj.begin())>;
    requires std::is_same_v<decltype(obj.data()), decltype(std::to_address(obj.begin()))>;
};

/**
 * @brief A utility class for inserting integal values with bit widths up to 64 bits into a Bitqueue-like container.
 *
 * This class writes data to a container that satisfies the `Bitqueue` concept, treating the container
 * as a contiguous stream of bits. It supports pushing single or multiple values of arbitrary bit widths.
 *
 * **Usage:**
 * - Instantiate the class with a `Bitqueue` container.
 * - Use the `push_back` methods to insert values of specific bit widths into the buffer.
 *
 * **Features:**
 * - Supports fixed-width pushes at compile-time (`push_back<B>`).
 * - Supports dynamic-width pushes at runtime (`push_back(std::size_t B)`).
 * - Handles both signed and unsigned integral types.
 * - Ensures that remaining buffered bits are flushed upon destruction.
 *
 * Example:
 * @code
 * Bitqueue_push_back pusher(bitqueue);
 * pusher.push_back<5>(value);  // Push 5 bits of value into the buffer
 * @endcode
 *
 */
class Bitqueue_push_back {
public:
    
    /**
     * @brief Constructor that initializes the Bitqueue_push_back instance with a writable data buffer.
     * @param container Bitqueue container that stores the data buffer to write to.
     */
    template <Bitqueue CONTAINER>
    Bitqueue_push_back(CONTAINER& container) noexcept : d_data(container.data()) {
        assert(&*container.begin() == d_data);
        assert((container.size() * sizeof(*d_data)) % sizeof(std::size_t) == 0 && "Container size must be a multiple of d_buffer size");
        assert(reinterpret_cast<std::uintptr_t>(d_data) % alignof(std::size_t) == 0 && "Container data must be aligned to the size of d_buffer");
    }
        
    /**
     * @brief Destructor that flushes the remaining bits to the data buffer.
     */
    ~Bitqueue_push_back() noexcept { flush_buffer(); }
    
    /**
     * @brief Relocates the writable data buffer to a new position.
     * @param data Pointer to the new writable data buffer.
     */
    constexpr void relocate(std::uint8_t* const data) noexcept { d_data = data; }

    /**
     * @brief flushes any buffered bits to memory
     */
    void flush_buffer() noexcept { std::memcpy(d_data, &d_buffer, sizeof(decltype(d_buffer))); }

    /**
     * @brief Retrieves the current writable data pointer.
     * @return Pointer to the current writable data buffer.
     */
    constexpr std::uint8_t* data() const noexcept { return d_data; }

    /**
     * @brief Pushes a single value of a compile-time fixed bit width into the buffer.
     * @tparam B Number of bits to push.
     * @tparam T Integral type of the value to push.
     * @param val The value to push into the buffer.
     */
    template <std::size_t B, std::integral T>
    constexpr void push_back(T const val) noexcept {
        if constexpr (B != 0)
            push_back<B>(std::span<T const,1>(&val, 1));
    }
    
    /**
     * @brief Pushes a single value of a specified bit width into the buffer.
     * @tparam T Integral type of the value to push.
     * @param B Number of bits to push.
     * @param val The value to push into the buffer.
     */
    template <std::integral T>
    constexpr void push_back(std::uint8_t const B, T const val) {
        if (B != 0)
            push_back(B, std::span<T const,1>(&val, 1));
    }
    
    /**
     * @brief Pushes multiple values of a compile-time fixed bit width into the buffer.
     * @tparam B Number of bits to push for each value.
     * @tparam T Integral type of the values to push.
     * @tparam N Size of the span containing the values.
     * @param values Span containing the values to push into the buffer.
     */
    template <std::uint8_t B, std::integral T, std::size_t N>
    constexpr void push_back(std::span<T,N> const values) noexcept {
        static_assert(sizeof(T) <= sizeof(std::size_t), "Type too large");
        if constexpr (B != 0) {
            decltype(d_buffer) const mask = ~decltype(d_buffer){0} >> (sizeof(decltype(d_buffer)) * 8 - B);
            for (auto const& val : values) {
                if constexpr (std::is_signed_v<T>)
                    d_buffer |= static_cast<decltype(d_buffer)>(static_cast<std::make_unsigned_t<T>>(val) & mask) << d_buffered_bits;
                else
                    d_buffer |= static_cast<decltype(d_buffer)>(val) << d_buffered_bits;
                d_buffered_bits += B;
                if (d_buffered_bits >= sizeof(decltype(d_buffer)) * 8) {
                    std::memcpy(d_data, &d_buffer, sizeof(decltype(d_buffer)));
                    d_data += sizeof(decltype(d_buffer));
                    d_buffered_bits -= sizeof(decltype(d_buffer)) * 8;
                    if constexpr (std::is_signed_v<T>)
                        d_buffer = static_cast<decltype(d_buffer)>(static_cast<std::make_unsigned_t<T>>(val) & mask) >> (B - d_buffered_bits);
                    else
                        d_buffer = static_cast<decltype(d_buffer)>(val) >> (B - d_buffered_bits);
                }
            }
        }
    }

    /**
     * @brief Pushes multiple values of a specified bit width into the buffer.
     * @tparam T Integral type of the values to push.
     * @tparam N Size of the span containing the values.
     * @param B Number of bits to push for each value.
     * @param values Span containing the values to push into the buffer.
     */
    template <std::integral T, std::size_t N>
    constexpr void push_back(std::uint8_t const B, std::span<T,N> const values) noexcept {
        static_assert(sizeof(T) <= sizeof(std::size_t), "Type too large");
        assert(B <= sizeof(T) * 8);
        if (B != 0) {
            decltype(d_buffer) const mask = ~decltype(d_buffer){0} >> (sizeof(decltype(d_buffer)) * 8 - B);
            for (auto const& val : values) {
                if constexpr (std::is_signed_v<T>)
                    d_buffer |= static_cast<decltype(d_buffer)>(static_cast<std::make_unsigned_t<T>>(val) & mask) << d_buffered_bits;
                else
                    d_buffer |= static_cast<decltype(d_buffer)>(val) << d_buffered_bits;
                d_buffered_bits += B;
                if (d_buffered_bits >= sizeof(decltype(d_buffer)) * 8) {
                    std::memcpy(d_data, &d_buffer, sizeof(decltype(d_buffer)));
                    d_data += sizeof(decltype(d_buffer));
                    d_buffered_bits -= sizeof(decltype(d_buffer)) * 8;
                    if constexpr (std::is_signed_v<T>)
                        d_buffer = static_cast<decltype(d_buffer)>(static_cast<std::make_unsigned_t<T>>(val) & mask) >> (B - d_buffered_bits);
                    else
                        d_buffer = static_cast<decltype(d_buffer)>(val) >> (B - d_buffered_bits);
                }
            }
        }
    }
    
    /**
     * @brief Computes the distance in bytes between the current buffer position and the given pointer.
     * @param p Pointer to compare the current position against.
     * @return Distance in bytes between the current position and the pointer.
     */
    constexpr std::ptrdiff_t operator-(std::uint8_t const* p) const noexcept { return d_data + (d_buffered_bits + 7) / 8 - p; }
    
private:
    std::uint8_t *d_data;
    std::uint8_t d_buffered_bits = 0;
    std::uint64_t d_buffer = 0;
};

/**
 * @brief A utility class for extracting integal values with bit widths up to 64 bits from a Bitqueue-like container.
 *
 * This class reads data from a container that satisfies the `Bitqueue` concept, interpreting the data as
 * a contiguous stream of bits. It supports popping single or multiple values of arbitrary bit widths.
 *
 * **Usage:**
 * - Instantiate the class with a `Bitqueue` container.
 * - Use the `pop` methods to extract values of specific bit widths from the buffer.
 * - Use the `skip` methods to skip over bits without extracting them.
 *
 * **Features:**
 * - Supports fixed-width pops at compile-time (`pop<B>`).
 * - Supports dynamic-width pops at runtime (`pop(std::size_t B)`).
 * - Handles both signed and unsigned integral types.
 *
 * Example:
 * @code
 * Bitqueue_pop popper(bitqueue);
 * auto value = popper.pop<5, std::uint8_t>();  // Extract 5 bits into a uint8_t
 * @endcode
 */
class Bitqueue_pop {
public:
    /**
     * @brief Constructor that initializes the Bitqueue_pop instance with data to process.
     * @param container Bitqueue container that stores the data buffer to read from.
     */
    template <Bitqueue CONTAINER>
    Bitqueue_pop(CONTAINER const& container) noexcept : d_data(container.data()) {
        assert(&*container.begin() == d_data);
        assert((container.size() * sizeof(*d_data)) % sizeof(std::size_t) == 0 && "Container size must be a multiple of d_buffer size");
        assert(reinterpret_cast<std::uintptr_t>(d_data) % alignof(std::size_t) == 0 && "Container data must be aligned to the size of d_buffer");
        std::memcpy(&d_buffer, d_data, std::min(container.size() * sizeof(*d_data), sizeof(std::size_t)));
    }
    /**
     * @brief Retrieves the current readable data pointer.
     * @return Pointer to the current readable data buffer.
     */
    constexpr std::uint8_t const* data() const noexcept { return d_data; }
    
    /**
     * @brief Pops a single value of specified bit width from the buffer.
     * @tparam B Number of bits to pop.
     * @tparam T Integral type to store the popped value.
     * @return The value popped from the buffer.
     */
    template <std::size_t B, std::integral T>
    constexpr T pop() noexcept {
        static_assert(sizeof(T) <= sizeof(decltype(d_buffer)), "Type too large");
        if constexpr (B == 0)
            return 0;
        else {
            T val;
            pop<B>(std::span<T,1>(&val, 1));
            return val;
        }
    }
    
    /**
     * @brief Pops a single value with a specified bit width from the buffer.
     * @tparam T Integral type to store the popped value.
     * @param B Number of bits to pop.
     * @return The value popped from the buffer.
     */
    template <std::integral T>
    constexpr T pop(std::uint8_t B) noexcept {
        static_assert(sizeof(T) <= sizeof(std::size_t), "Type too large");
        if (B == 0)
            return 0;
        else {
            T val;
            pop(B, std::span<T,1>(&val, 1));
            return val;
        }
    }
    
    /**
     * @brief Pops multiple values of a specified bit width from the buffer.
     * @tparam T Integral type to store the popped values.
     * @tparam N Size of the span for storing the values.
     * @param B Number of bits to pop for each value.
     * @param values Span to store the popped values.
     */
    template <std::integral T, std::size_t N>
    constexpr void pop(std::uint8_t const B, std::span<T,N> values) noexcept {
        assert(B <= 8 * sizeof(T)); // More bits requested than depth of T: B too large
        if (B==0)
            std::fill(values.begin(), values.end(), T{0});
        else {
            T const mask = static_cast<T>(~std::uint64_t{0} >> (64 - B));
            T const is_negative = std::is_unsigned_v<T> ? T(0) : T(T(1) << (B - 1));
            for (auto& p : values) {
                p = static_cast<T>(d_buffer);
                if (B <= d_buffered_bits){
                    if (B == 8 * sizeof(decltype(d_buffer)))
                        d_buffer = d_buffered_bits = 0;
                    else {
                        d_buffer >>= B;
                        d_buffered_bits -= B;
                    }
                }
                else {
                    d_data += sizeof(decltype(d_buffer));
                    std::memcpy(&d_buffer, d_data, sizeof(decltype(d_buffer)));
                    p |= static_cast<T>(d_buffer << d_buffered_bits);
                    d_buffer >>= B - d_buffered_bits;
                    d_buffered_bits = sizeof(decltype(d_buffer)) * 8 + d_buffered_bits - B;
                }
                if (B != 8 * sizeof(T)) {
                    if (p & is_negative) p |= ~mask;
                    else p &= mask;
                }
            }
        }
    }
    
    /**
     * @brief Pops multiple values of a compile-time fixed bit width from the buffer.
     * @tparam B Number of bits to pop.
     * @tparam T Integral type to store the popped values.
     * @tparam N Size of the span for storing the values.
     * @param values Span to store the popped values.
     */
    template <std::uint8_t B, std::integral T, std::size_t N> requires (!std::is_same_v<T, bool>)
    constexpr void pop(std::span<T,N> values) noexcept {
        assert(B <= 8 * sizeof(T)); // More bits requested than depth of T: B too large
        if constexpr (B==0)
            std::fill(values.begin(), values.end(), T{0});
        else {
            for (auto& p : values) {
                p = static_cast<T>(d_buffer);
                if (B <= d_buffered_bits){
                    if constexpr (B == 8 * sizeof(decltype(d_buffer)))
                        d_buffered_bits = d_buffer = 0;
                    else {
                        d_buffer >>= B;
                        d_buffered_bits -= B;
                    }
                }
                else {
                    d_data += sizeof(decltype(d_buffer));
                    std::memcpy(&d_buffer, d_data, sizeof(decltype(d_buffer)));
                    p |= static_cast<T>(d_buffer << d_buffered_bits);
                    d_buffer >>= B - d_buffered_bits;
                    d_buffered_bits = sizeof(decltype(d_buffer)) * 8 + d_buffered_bits - B;
                }
                if constexpr (B != 8 * sizeof(T)) {
                    constexpr T is_negative = std::is_unsigned_v<T> ? T(0) : T(T(1) << (B - 1));
                    constexpr auto mask = static_cast<std::make_unsigned_t<T>>(~std::uint64_t{0} >> (64 - B));
                    if (p & is_negative) p |= static_cast<T>(~mask);
                    else p &= static_cast<T>(mask);
                }
            }
        }
    }
    
    /**
     * @brief Skips a fixed number of bits in the buffer.
     * @tparam B Number of bits to skip.
     */
    template <std::size_t B>
    constexpr void skip() {
        if (B <= d_buffered_bits)
            d_buffered_bits -= static_cast<std::uint8_t>(B);
        else {
            d_data += sizeof(std::size_t) + B / (sizeof(std::size_t) * 8) ;
            d_buffered_bits = (sizeof(std::size_t) * 8) - (B % (sizeof(std::size_t) * 8));
        }
    }
    
    /**
     * @brief Skips a specified number of bits in the buffer.
     * @param B Number of bits to skip.
     */
    constexpr void skip(std::size_t const B) {
        if (B <= d_buffered_bits)
            d_buffered_bits -= static_cast<std::uint8_t>(B);
        else {
            d_data += sizeof(std::size_t) + B / (sizeof(std::size_t) * 8) ;
            d_buffered_bits = (sizeof(std::size_t) * 8) - (B % (sizeof(std::size_t) * 8));
        }
    }
    
    /**
     * @brief Computes the distance in bytes between the current buffer position and the given pointer.
     * @param p Pointer to compare the current position against.
     * @return Distance in bytes between the current position and the pointer.
     */
    constexpr std::ptrdiff_t operator-(std::uint8_t const* p) const noexcept { return d_data + (d_buffered_bits + 7) / 8 - p; }

private:
    std::uint8_t const* d_data;
    std::size_t d_buffer;
    std::uint8_t d_buffered_bits = sizeof(decltype(d_buffer)) * 8;
};

}// end namespace jpa

#endif /* Bitqueue_h */
