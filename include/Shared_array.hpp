//
//  Shared_array.hpp
//  Shared_array
//
//  Created by Jan Pieter Abrahams on 23.01.2024.
//
//    The `Shared_array` class is a light-weight C++ container designed for managing arrays with shared ownership
//    semantics, combining aspects of `std::shared_ptr` and `std::span`. It acts as a smart pointer for arrays,
//    offering automatic memory management with shared ownership, alongside convenient array-like access. This class
//    is particularly useful in scenarios where multiple parts of a program need to manage and access the same array
//    safely and efficiently.
//
//    ### Key Features:
//    1. **Shared Ownership**: Unlike `Unique_array` or 'std::vector', `Shared_array` allows multiple instances to
//    share ownership of the underlying array. The array is automatically deallocated when the last `Shared_array`
//    owning it goes out of scope, thus preventing memory leaks.
//
//    2. **Slicing and Viewing Data Structure**: This feature complements the shared ownership model by allowing
//    users to create slices or views of the array. These views can access specific segments of the data without
//    duplication, all while maintaining the guarantee that the underlying memory won't be released as long
//    as any view or slice remains active. This ensures safety and consistency when accessing subsets of the array.
//
//    3. **Array-like Access**: Inheriting from `std::span`, `Shared_array` provides direct, bounds-checked access
//    to its elements, similar to standard C++ arrays and containers like `std::vector`.
//
//    4. **Support for Dynamic and Fixed-Size Arrays**: The class template can be used for both dynamic arrays
//    (where the size is determined at runtime) and fixed-size arrays (with compile-time size).
//
//    5. **Copy and Move Semantics**: `Shared_array` supports both copying and moving, making it flexible for
//    use in a variety of contexts where shared data management is needed.
//
//    6. **Reinterpreting Data**: This feature complements the shared ownership model by allowing users to reinterpret
//    data, for instance, to access a Shared_array of bytes as a Shared_array of unsigned longs.
//
//    ### Use Cases:
//    - **Collaborative Data Management**: Ideal for applications where multiple components or threads need to
//    access and potentially modify the same array data.
//
//    - **Complex Data Structures in Multi-Threaded Environments**: Useful for managing shared resources in
//    multi-threaded applications, facilitating safe and efficient access to shared data.
//
//    - **Slicing and Viewing Data Structure**: Useful for extracting slices or views that allow access to common
//    data, with the guarantee that associated memory is not released until all views or slices have gone out of
//    scope.
//
//    - **Graphs and Trees with Shared Nodes**: In data structures like graphs or trees, `Shared_array` can
//    efficiently manage arrays of shared nodes or edges.
//
//    - **Cache and Buffer Sharing**: For scenarios where different parts of an application need to share access
//    to a common cache or buffer.
//
//    ### Example:
//      jpa::Shared_array<double> sharedData1(1000); // Shared array of 1000 doubles
//      auto sharedData2 = sharedData1; // sharedData2 now shares ownership with sharedData1
//
//      // Both sharedData1 and sharedData2 can access and modify the array
//      for (int i = 0; i < 1000; ++i) {
//          sharedData1[i] = computeSomeValue(i);
//      }
//
//    In this example, `Shared_array` is used to create a shared array of doubles. Modifications through
//    either reference will be reflected in the other. The array's lifetime is managed through shared ownership,
//    ensuring that it exists as long as any `Shared_array` instance owns it.
//
//    In summary, `Shared_array` is a valuable tool for managing arrays in C++, particularly useful in shared
//    ownership scenarios. Its design offers a balance between safety, efficiency, and shared data management,
//    suitable for a variety of applications, especially those involving collaborative data access or complex
//    data structures.

#ifndef SHARED_ARRAY_HPP
#define SHARED_ARRAY_HPP

#include <memory>
#include <span>
#include <array>
#include <type_traits>

namespace jpa {

/// \brief A template class for managing a shared array.
/// \details Shared_array combines shared ownership (like std::shared_ptr) with array-like access (like std::span).
///          It can manage either dynamic arrays (with std::dynamic_extent) or fixed-size arrays.
/// \tparam T The type of the elements in the array.
/// \tparam N The size of the array, defaults to std::dynamic_extent for a dynamic array.
template <typename T, std::size_t N = std::dynamic_extent>
class Shared_array :
private std::shared_ptr<std::byte[]>,
public std::span<T, N> {
    template <typename, std::size_t> friend class Shared_array;
    static_assert(std::is_default_constructible_v<T>, "Type T must be default constructible for Shared_array");
public:
    /// \brief Constructor for a dynamic array.
    /// \param size The size of the array to create.
    /// \note This constructor is only available for dynamic arrays (N == std::dynamic_extent).
    explicit Shared_array(std::size_t size) noexcept requires (N == std::dynamic_extent) :
    std::shared_ptr<std::byte[]>(new std::byte[size * sizeof(T)]),
    std::span<T, N>(reinterpret_cast<T*>(this->get()), size) {}
    
    /// \brief Default constructor for a fixed-size array.
    /// \note This constructor is only available for fixed-size arrays (N != std::dynamic_extent).
    constexpr Shared_array() noexcept requires (N != std::dynamic_extent) :
    std::shared_ptr<std::byte[]>(new std::byte[N * sizeof(T)]),
    std::span<T, N>(reinterpret_cast<std::array<T,N>&>(*this->get())) {}
    
    /// \brief Constructor to create a dynamic Shared_array from another fixed Shared_array of a different type.
    /// \tparam U The type parameter of the other Shared_array.
    /// \param other The other Shared_array.
    template<typename U>
    constexpr Shared_array(Shared_array<U,N> const& other) noexcept requires (N == std::dynamic_extent) :
    std::shared_ptr<std::byte[]>(other),
    std::span<T, N>(reinterpret_cast<T*>(this->get()), other.size() * sizeof(U) / sizeof(T)) {
        static_assert(sizeof(U) % sizeof(T) == 0 || sizeof(T) % sizeof(U) == 0,
                      "Incompatible type sizes for Shared_array reinterpretation.");
    }
    
    /// \brief Constructor to create a fixed-size  Shared_array from another fixed Shared_array of a different type.
    /// \tparam U The type parameter of the other Shared_array.
    /// \param other The other Shared_array.
    template<typename U, std::size_t M>
    constexpr Shared_array(Shared_array<U,M> const& other) noexcept requires (N != std::dynamic_extent) :
    std::shared_ptr<std::byte[]>(other),
    std::span<T, N>(reinterpret_cast<std::array<T,N>&>(*this->get())) {
        static_assert(N * sizeof(T) ==  M * sizeof(U),
                      "Incompatible type sizes for Shared_array reinterpretation.");
    }
    
    /// Shared_array is copyable and movable.
    constexpr Shared_array(const Shared_array&) = default;
    constexpr Shared_array(Shared_array&&) = default;
    constexpr Shared_array& operator=(const Shared_array&) = default;
    constexpr Shared_array& operator=(Shared_array&&) = default;
    
    using std::span<T, N>::operator[];
    
    /// \brief Checks if the current instance is the only owner of the array.
    /// \return True if this is the only owner, false otherwise.
    constexpr bool is_unique() const noexcept {
        return use_count() == 1;
    }
    
    /// \brief Creates a slice of the Shared_array
    /// \param offset The starting index of the slice
    /// \param length The number of elements in the slice
    /// \return A new Shared_array instance that shares ownership of the array but only accesses a subset of it.
    constexpr Shared_array slice(std::size_t offset, std::size_t length) const {
        if (offset + length > this->size())
            throw std::out_of_range("Slice range is out of bounds");
        return Shared_array(reinterpret_cast<const std::shared_ptr<std::byte[]>&>(*this), this->data() + offset, length);
    }
    
    /// \brief Creates a fixed-sized slice of the Shared_array
    /// \param offset The starting index of the slice
    /// \tparam M The number of elements in the slice
    /// \return A new fixed-size Shared_array instance that shares ownership of the array but only accesses a subset of it.
    template <std::size_t M>
    constexpr Shared_array<T,M> slice(std::size_t offset) const {
        if (offset + M > this->size())
            throw std::out_of_range("Slice range is out of bounds");
        return Shared_array<T,M>(reinterpret_cast<const std::shared_ptr<std::byte[]>&>(*this), this->data() + offset);
    }
     
private:
    // Private constructor for creating dynamic slices
    constexpr Shared_array(const std::shared_ptr<std::byte[]>& sharedArray, T* slicedPtr, std::size_t length) requires (N == std::dynamic_extent) :
    std::shared_ptr<std::byte[]>(sharedArray, reinterpret_cast<std::byte*>(slicedPtr)),
    std::span<T, N>(slicedPtr, length) {}

    // Private constructor for creating dynamic slices
    constexpr Shared_array(const std::shared_ptr<std::byte[]>& sharedArray, T* slicedPtr) requires (N != std::dynamic_extent) :
   std::shared_ptr<std::byte[]>(sharedArray, reinterpret_cast<std::byte*>(slicedPtr)),
   std::span<T, N>(reinterpret_cast<std::array<T,N>&>(*this->get())) {}
};

} // namespace jpa


#endif /* SHARED_ARRAY_HPP */
