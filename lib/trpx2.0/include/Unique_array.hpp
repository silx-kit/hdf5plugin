//
//  Unique_array.hpp
//  Unique_array
//
//  Created by Jan Pieter Abrahams on 23.01.2024.
//
//    The `Unique_array` class is a light-weight C++ container designed for managing arrays with unique ownership
//    semantics, combining aspects of `std::unique_ptr` and `std::span`. It is essentially a smart pointer for
//    arrays, offering automatic memory management alongside convenient array-like access. This class is
//    particularly useful in scenarios where you need the dynamic allocation of an array with the safety and
//    functionality of a standard container.
//
//    ### Key Features:
//    1. **Unique Ownership**: Like `std::unique_ptr`, `Unique_array` ensures that there is only one owner of
//    the underlying array at any time. This guarantees that the array is automatically deallocated when the
//    `Unique_array` object goes out of scope, preventing memory leaks.
//
//    2. **Array-like Access**: Thanks to inheriting from `std::span`, `Unique_array` provides direct,
//    bounds-checked access to its elements, similar to standard C++ arrays and containers like `std::vector`.
//
//    3. **Support for Dynamic and Fixed-Size Arrays**: The class template can be instantiated for either
//    dynamic arrays (where the size is determined at runtime) or fixed-size arrays (with compile-time size).
//
//    4. **Move Semantics**: `Unique_array` supports move semantics, allowing efficient transfer of ownership 
//    of the array between `Unique_array` instances.
//
//    5. **No Copy Semantics**: In line with unique ownership, copying of `Unique_array` instances is not
//    allowed, preventing unintentional duplication of array ownership.
//
//    ### Use Cases:
//    - **Resource Management in High-Level Code**: When dealing with raw dynamic arrays in C++, there is 
//    always a risk of memory leaks or dangling pointers. `Unique_array` encapsulates this complexity, ensuring
//    that memory is automatically managed, making it a safer choice for managing dynamically allocated arrays.
//
//    - **Performance-Critical Applications**: In scenarios where performance is critical, such as real-time
//    systems or high-performance computing, the deterministic destruction and lack of overhead in copying that
//    `Unique_array` provides can be very beneficial.
//
//    - **Safe Array Manipulation**: For applications that require safe access to array elements, `Unique_array`
//    offers bounds-checked access, reducing the risk of out-of-bounds errors.
//
//    - **Complex Data Structures**: In applications involving complex data structures, such as trees or graphs
//    with dynamically allocated nodes, `Unique_array` can manage arrays of pointers or objects efficiently,
//    ensuring proper cleanup.
//
//    - **Scientific Computing and Image Processing**: In fields like scientific computing or image processing,
//    where large arrays of data are common, `Unique_array` provides an efficient way to handle these arrays with
//    automatic memory management and easy access.
//
//    ### Example:
//      jpa::Unique_array<double> data(1000); // Dynamically allocated array of 1000 doubles
//      for (int i = 0; i < 1000; ++i) {
//          data[i] = computeSomeValue(i); // Safe access to elements
//      }
//
//    In this example, `Unique_array` is used to create a dynamically allocated array of doubles. The array is
//    automatically deallocated when it goes out of scope, preventing memory leaks.
//
//    Advantages of Using Unique_array over Direct new T[] and new std::array<T, N>:
//
//    ### Safety and Automatic Memory Management:
//    - **Automatic Deallocation**: `Unique_array` automatically deallocates the array when it goes out of scope,
//      preventing memory leaks, which is a common issue with manual memory management using `new` and `delete`.
//    - **Exception Safety**: `Unique_array` provides strong exception safety. If an exception is thrown after
//      the array is allocated, `Unique_array` ensures proper cleanup, a challenge with raw pointers.
//
//    ### Modern C++ Best Practices:
//    - **Resource Management**: Following the RAII (Resource Acquisition Is Initialization) idiom,
//      `Unique_array` ensures that resources are tied to the lifespan of objects, leading to more predictable
//      and maintainable code.
//    - **Avoiding Raw Pointers**: Modern C++ guidelines advise against using raw pointers for ownership and
//      memory management. `Unique_array` encapsulates these best practices.
//
//    ### Ease of Use and Convenience:
//    - **Array-like Access**: Provides a familiar array-like interface, including element access via the `[]`
//      operator and bounds checking (when using `std::span`).
//    - **Compatibility with Standard Library**: Easier integration with C++ standard library algorithms and
//      functions compared to raw pointers.
//    - **Size Retrieval**: Unlike raw dynamic arrays, `Unique_array` allows for easy retrieval of the array
//      size, which is crucial for safe array operations and iteration.
//
//    ### Flexibility:
//    - **Move Semantics**: Supports move semantics, enabling efficient transfer and ownership management,
//      not directly available with raw array pointers.
//    - **Support for Dynamic and Fixed-Size Arrays**: Useful for both dynamically allocated and fixed-size
//      arrays, providing a unified interface.
//
//    In summary, `Unique_array` is a versatile and safe container for managing arrays in C++, particularly
//    useful when both unique ownership and array-like access are desired. Its design simplifies memory
//    management and enhances code safety and clarity, especially in complex or performance-critical
//    applications. `Unique_array` is recommended over direct use of `new T[]` and `new std::array<T, N>`,
//    as it leads to safer, more readable, and maintainable code.

#ifndef UNIQUE_ARRAY_HPP
#define UNIQUE_ARRAY_HPP

#include <memory>
#include <span>
#include <iostream>

namespace jpa {

/// \brief A template class for managing a unique array.
///        Can be either dynamic (with std::dynamic_extent) or fixed-size.
/// \tparam T Type of the elements in the array.
/// \tparam N Size of the array. Defaults to std::dynamic_extent for a dynamic array.
template <typename T, std::size_t N = std::dynamic_extent>
class Unique_array :
private std::unique_ptr<std::conditional_t<N == std::dynamic_extent, T[], std::array<T,N>>>,
public std::span<T,N> {
public:
    /// Deleted copy constructor.
    Unique_array(Unique_array const&) = delete;
    
    /// Deleted copy assignment operator.
    Unique_array& operator=(const Unique_array&) = delete;
    
    /// Constructor for dynamic array.
    /// \param size The size of the array to create.
    Unique_array(std::size_t size) noexcept requires (N == std::dynamic_extent) :
    std::unique_ptr<T[]>(std::make_unique<T[]>(size)),
    std::span<T,N>(this->get(), size)
    {}
    
    /// Default constructor for fixed-size array.
    Unique_array() noexcept requires (N != std::dynamic_extent) :
    std::unique_ptr<std::array<T,N>>(new std::array<T,N>),
    std::span<T,N>(*this->get())
    {}
    
    /// Move constructor for dynamic array.
    /// \param other Another Unique_array to move from.
    Unique_array(Unique_array&& other) noexcept requires (N == std::dynamic_extent) :
    std::unique_ptr<T[]>(std::move((other))),
    std::span<T,N>(this->get(), other.size())
    {}
    
    /// Move constructor for fixed-size array.
    /// \param other Another Unique_array to move from.
    Unique_array(Unique_array&& other) noexcept requires (N != std::dynamic_extent) :
    std::unique_ptr<std::array<T,N>>(std::move(other)),
    std::span<T,N>(*this->get())
    {}
    
    /// Move assignment operator.
    /// \param other Another Unique_array to assign from.
    /// \return Reference to this Unique_array.
    Unique_array& operator=(Unique_array&& other) noexcept {
        if (this != &other) {
           std::unique_ptr<std::conditional_t<N == std::dynamic_extent, T[], std::array<T, N>>>::operator=(std::move(other));
            if constexpr (N == std::dynamic_extent)
                this->std::span<T, N>::operator=(std::span<T, N>(this->get(), other.size()));
            else
                this->std::span<T, N>::operator=(std::span<T, N>(*this->get()));
        }
        return *this;
    }

    using std::span<T, N>::operator[];
};

/// Compare two Unique_array objects for equality.
/// \param lhs Left-hand side Unique_array to compare.
/// \param rhs Right-hand side Unique_array to compare.
/// \return True if equal, false otherwise.
template <typename T, std::size_t N>
bool operator==(Unique_array<T,N> const& lhs, Unique_array<T,N> const& rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

/// Compare two Unique_array objects using three-way comparison.
/// \param lhs Left-hand side Unique_array to compare.
/// \param rhs Right-hand side Unique_array to compare.
/// \return std::strong_ordering result of the comparison.
template <typename T, std::size_t N>
std::strong_ordering operator<=>(const Unique_array<T,N>& lhs, const Unique_array<T,N>& rhs) {
    for (std::size_t i = 0; i < lhs.size() && i < rhs.size(); ++i)
        if (auto cmp = lhs[i] <=> rhs[i]; cmp != 0) return cmp;
    return lhs.size() <=> rhs.size();
}

} // namespace jpa

#endif /* UNIQUE_ARRAY_HPP */
