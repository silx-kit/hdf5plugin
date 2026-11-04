//
//  Grey_tif.h
//  Grey_tif
//
//  Created by Jan Pieter Abrahams on 23.05.23.
//

#ifndef Grey_tif_h
#define Grey_tif_h


#include <iostream>
#include <array>
#include <vector>
#include <span>
#include <cassert>
#include <type_traits>
#include <algorithm>
#include <bit>


// DISCLAIMER: This is not a general-purpose TIFF library!

namespace jpa {
/**
 * @brief Grey_tif: A specialized  class for  two-dimensional scientific greyscale TIFF files, images & stacks
 *
 * Key Features:
 * - Reads TIFF files that contain a single image or a stack of images.
 * - Seamless  appending and extraction of image data from/to TIFF files, STL containers and other Grey_tif objects.
 * - Compact API that is powerful, intuitive, flexible and easy-to-learn/use.
 * - Image data of any image on the stack can be accessed through std::span, an STL compatible range view.
 * - Full read and write access to all pixel values.
 * - Multi-dimensional subscript operator (C++23 feature) for convenient indexing.
 * - Compact code, optimized for high-speed performance and minimal memory foorprint.
 * - Accommodating images with varying types and sizes.
 * - Support for signed and unsigned integral values, and float and double values.
 * - Flexible bit depths: 8, 16, 32, or (for double precision values) 64 bits.
 * - Accomodates raw TIFF data with runtinme pixel type identification.
 * - Allows regularizing pixel types to a compile-time determined type.
 * - The first (or only) image managed by a Grey_tif object is directly available as a readonly std::span
 *
 * Limitations: Grey_tif does not support:
 * - TIFF files with fragmented images.
 * - TIFF files with single-bit (black & white) or color images.
 * - TIFF files that contain compressed images.
 *
 * The Grey_tif<T>  API has 7 member functions:
 * - A default constructor() and constructors using a TIFF stream or an STL container.
 * - write(): write a TIFF file to a stream.
 * - clear(): remove all images.
 * - swap(): swap image contants with another Grey_tif object, reacsting the pixel types of both Grey_tif containers if required.
 * - image_stack_size(): get the number of images managed by the Grey_tif object.
 * - push_back(): append one or more images from an STL container or other Grey_tif object.
 * - const_image(): returns a const Grey_tif_image from the tif stack for readonly access to images form a TIFF stack.
 * - image(): returns a Grey_tif_image object for read/write access to images form a TIFF stack.
 *
 * The Grey_tif_image  API is publicly derived from std::span, and has two additional member functions:
 * - type(): returns a POD_type_traits object providing information about the size, signedness, and integral nature of the image's pixel type.
 * - operator[int, int]: the multi-dimensional subscript operator (C++23 feature) is available if supported by the compiler.
 *
 * Example that tests all features of Grey_tif.hpp:
 *
 * @code{.cpp}
 * #include <fstream>
 * #include <filesystem>
 * #include "Grey_tif.hpp"
 *
 * int main(int argc, const char * argv[]) {
 *     using namespace jpa;
 *
 *     // Generate a Grey_tif<int> object from a container
 *     std::vector<std::uint16_t> img0({42,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
 *     Grey_tif<int> tif(img0, {4, 4});
 *
 *     // The pixel type is defaults to the type of the initializing container
 *     Grey_tif tif_uint16(img0, {4, 4});
 *     assert (tif.type().is<int>() && tif_uint16.type().is<uint16_t>());
 *
 *     // Append a Grey_tif object using a TIFF file; pixel format of all images is int, regardless of
 *     // the encoding in the TIFF file
 *     std::ifstream istream("/Users/abrahams/Dropbox/develop/jpa/Grey_tif/Grey_tif/infile.tif", std::ios::binary);
 *     tif.push_back(Grey_tif(istream));
 *
 *     // Readonly access of pixels of the first (or only) image of a Grey_tif as an STL range view (a std::span)
 *     assert(tif[0] == 42);
 *
 *     // Read/write access is achieved through the image() member function, for constant images, use const_image()
 *     tif.image(0)[0] = tif.image(1)[0] = 43;
 *     // tif.const_image(1) = 44; <--- does not compile
 *     assert (tif.const_image(1)[0] == 43 && tif.image(1)[0] == 43);
 *
 *
 *     // A Grey_tif object can be written to disk using its 'write(ostream)' member function.
 *     std::filesystem::path output = "/Users/abrahams/Dropbox/develop/jpa/Grey_tif/Grey_tif/outfile.tif";
 *     std::ofstream ostream(output, std::ios::binary);
 *     tif.write(ostream);
 *
 *     // Raw Grey_tif objects can storore images with different pixel types
 *     Grey_tif<std::byte> tiff_raw(img0, {4, 4});
 *     tiff_raw.push_back<std::int32_t>({10,12}); // You can also pushback a zero-initialized image by only providing its dimensions
 *
 *     // Access to pixel data of raw Grey_tif only possible with the correct template parameter
 *     // assert(tif[0] == 42 && tif.image(0)[0] == 42); <--- does not compile
 *     assert(tiff_raw.image<unsigned short>(0)[0] == 42);
 *     assert((static_cast<Grey_tif_image<unsigned short const>>(tiff_raw))[0] == 42); // alternatively, a typecast is possible.
 *
 *     // Grey_tif objects can be swapped. If a raw Grey_tif is swapped with a typed Grey_tiff, type conversion
 *     // of the pixel values occurs
 *     Grey_tif<float> tif_float; // construct an empty Grey_tif object with float pixel values
 *     tif_float.swap(tiff_raw); // swapping converts the pixel values of all raw images from tif_raw to float
 *     tif_float.swap(tiff_raw); //swapping back preserves the new pixel values as float
 *     assert(tiff_raw.image<float>(0).size() == 16);
 *
 *     // tif objects can also be cleared:
 *     assert(tif.image_stack_size() == 2);
 *     assert(tif.clear().image_stack_size() == 0);
 * }
 * @endcode
 *
 * @tparam T The default / assumed pixel data type (default: std::uint16_t).
 */
template <typename T = std::uint16_t> class Grey_tif;

/**
 * @brief POD_type_traits represents traits information about Plain Old Data (POD) types.
 *
 * This struct provides information about the size, signedness, and integral nature of a type.
 */
struct POD_type_traits {
    /**
     * @brief The size in bytes of the type.
     */
    std::size_t size;
    
    /**
     * @brief Flag indicating whether the type is signed.
     */
    bool is_signed;
    
    /**
     * @brief Flag indicating whether the type is integral.
     */
    bool is_integral;
    
    /**
     * @brief Check if  type T matches the specified criteria.
     *
     * This function checks if a given type T matches the size, signedness, and integral nature
     * criteria specified by this POD_type_traits object.
     *
     * @tparam T The type to check.
     * @return True if the type matches the criteria, false otherwise.
     */
    template <typename T>
    bool is() const { return sizeof(T) == size && std::is_integral_v<T> == is_integral && std::is_signed_v<T> == is_signed; }
    
    /**
     * @brief Compare this POD_type_traits object with another for equality.
     *
     * @param other The POD_type_traits object to compare with.
     * @return True if the objects are equal, false otherwise.
     */
    bool operator ==(POD_type_traits const& other) const noexcept {
        return size == other.size && is_signed == other.is_signed && is_integral == other.is_integral;
    }
};

/**
 * @brief API to an image managed by a regularized Grey_tif object with a compile time defined pixel type.
 *
 * This class provides methods for manipulating and accessing pixel data of an image in a Grey_tif object. It is an STL compatible range view, and inherits and provides all the features of std::span.
 * It has no public constructors and can only be created by a Grey_tif object to allow access to its TIFF image data. If the In case of raw TIFF data (stored in a Grey_tif<std::byte> object), the size() member function returns zero if the template parameter does not correspond to the type encoded in the raw TIFF data.
 *
 * @tparam T The  type of pixel data in the image.
 * @see std::span
 * @see Grey_tif
 */
template<typename T>
class Grey_tif_image : public std::span<T> {
    
    template <typename TG> friend class Grey_tif;
    template <typename TG> friend class Grey_tif_image;
    
public:
    
#ifdef __cpp_multidimensional_subscript
    /**
     * @brief Accessor for image data using multidimensional subscripting.
     *
     * Allows read-only access to individual pixel values within the TIFF images using multidimensional subscripting without bounds checking.
     *
     * @param i0 The first dimension index.
     * @param i1 The second dimension index.
     * @return A reference to the pixel value at the specified indices.
     */
    constexpr auto const& operator [] (std::size_t i0, std::size_t i1) const noexcept { return (*this)[d_dim[0] * i0 + i1]; }
    
    /**
     * @brief Accessor for image data using multidimensional subscripting.
     *
     * Allows read/write access to individual pixel values within the TIFF images using multidimensional subscripting without bounds checking.
     *
     * @param i0 The first dimension index.
     * @param i1 The second dimension index.
     * @return A reference to the pixel value at the specified indices.
     */
    constexpr auto      & operator [] (std::size_t i0, std::size_t i1)       noexcept { return (*this)[d_dim[0] * i0 + i1]; }

    /**
     * @brief Accessor for image data using one-dimensional subscripting.
     *
     * Allows read-only access to individual pixel values within the TIFF images using single dimenion subscripting without bounds checking.
     *
     * @param i0 The first dimension index.
     * @return A reference to the pixel value at the specified indices.
     */
    constexpr auto const& operator [] (std::size_t i0) const noexcept {return std::span<T>::operator[](i0);}

    /**
     * @brief Accessor for image data using one-dimensional subscripting.
     *
     * Allows read/write access to individual pixel values within the TIFF images using single dimenion subscripting without bounds checking.
     *
     * @param i0 The first dimension index.
     * @return A reference to the pixel value at the specified indices.
     */
    constexpr auto      & operator [] (std::size_t i0) noexcept {return std::span<T>::operator[](i0);}
#endif
    
    /**
     * @brief Get the dimensions of the  TIFF image.
     *
     * @return An array containing the width and height of the image.
     */
    std::array<long,2> const& dim() const noexcept { return d_dim; }
    
    /**
     * @brief Get the pixel data type of the  TIFF image.
     *
     * @return The pixel data type of the image as a POD_type_traits.
     * @see POD_type_traits
     */
    POD_type_traits const& type() const noexcept {return d_image_type;}
    
private:
    Grey_tif_image(POD_type_traits const& image_type, std::array<long,2> const& dim, std::span<T> const& data) :
    std::span<T>(data),
    d_image_type(image_type),
    d_dim(dim) {}
    
    POD_type_traits d_image_type;
    std::array<long,2> d_dim;
};


/**
 * @brief API to image parameters of a raw Grey_tif<std::byte> object with a runtime defined pixel type.
 *
 * This class allows determining the runtime pixel type and dimensions of an image managed by a raw Grey_tif<std::byte> object.
 * But it does not allow access to the pixel values. In order to address its pixel values, it must first be cast to
 * a Grey_tif_image of the correct pixel type T, which is the case when raw_tif.type().is<T>() is true.
 * It has no public constructors and can only be created by a Grey_tif<std::byte> object. The size() member function returns zero.
 */
template<typename T> requires std::is_same_v<std::remove_const_t<T>, std::byte>
class Grey_tif_image<T> {
    
    template <typename TG> friend class Grey_tif;
    
public:
    /**
     * @brief Always returns zero to indicate that the image pixel type must be determined at runtime.
     *
     * @return zero.
     */
    std::size_t size() const noexcept { return 0; }
    
    /**
     * @brief Get the dimensions of the raw TIFF image.
     *
     * @return An array containing the width and height of the image.
     */
    std::array<long,2> const& dim() const noexcept { return d_dim; }
    
    /**
     * @brief Get the pixel data type of the  TIFF image.
     *
     * @return The pixel data type of the image as a POD_type_traits.
     * @see POD_type_traits
     */
    POD_type_traits const& type() const noexcept {return d_image_type;}
    
    /**
     * @brief Typecasting a Grey_tif_image to its runtime POD type
     *
     * Grey_tif_image<std::byte const> and Grey_tif_image<std::byte> can only be typecast to a type that corresponds for which type().is<T>() is true.
     *
     * @tparam T0 POD type that is encoded by the POD type as returned by type().
     * @return Grey_tif_image<T>
     */
    template <typename T0>
    operator Grey_tif_image<T0>() const {
        static_assert ((std::is_integral_v<T0> && sizeof(T0) <= 4 && !std::is_same_v<T0, bool>) ||
                       (std::is_floating_point_v<T0> && sizeof(T0) <= 8),
                       "Only signed & unsigned 8-, 16 and 32-bit integers, floats and doubles allowed in TIFF image data");
        if constexpr (std::is_const_v<T>)
            static_assert(std::is_const_v<T0>, "Can only cast a Grey_tif_image<std::byte const> to a Grey_tif_image<T>, when T is const");
        assert(type().template is<T0>());
        return Grey_tif_image<T0>(type(), dim(), std::span<T0>(reinterpret_cast<T0*>(d_data), dim()[0] * dim()[1]));
    }
    
private:
    Grey_tif_image(POD_type_traits const& image_type, std::array<long,2> const& dim, std::span<T> const& data) :
    d_data(data.size() > 0 ? reinterpret_cast<T*>(&data.begin()[0]) : nullptr),
    d_image_type(image_type),
    d_dim(dim) { }

    T* d_data;
    POD_type_traits d_image_type;
    std::array<long,2> d_dim;
};

template <typename T>
class Grey_tif : public Grey_tif_image<T const> {
    static_assert ((std::is_integral_v<T> && sizeof(T) <= 4 && !std::is_same_v<T, bool>) ||
                   std::is_same_v<T, std::byte> ||
                   (std::is_floating_point_v<T> && sizeof(T) <= 8),
                   "Only signed & unsigned 8-, 16 and 32-bit integers, floats and doubles allowed in TIFF image data");
    
    template <typename T0> friend class Grey_tif;
    
public:
    using value_type = T;
    
    /**
     * @brief Default constructor for Grey_tif with no images.
     */
    Grey_tif() noexcept :
    Grey_tif_image<T const>(POD_type_traits{.size = sizeof(T), .is_signed = std::is_signed_v<T>, .is_integral = std::is_integral_v<T>},
                            {0, 0}, std::span<T const>()) {
        d_tif = std::vector<std::byte>(8, std::byte(0x00));
        d_tif[0] = d_tif[1] = std::byte((std::endian::native == std::endian::little) ? 'I' : 'M');
        reinterpret_cast<std::uint16_t&>(d_tif[2]) = 42;
        d_last_ifd_offset = 4;
    }
    
    /**
     * @brief Constructor for Grey_tif of a single image with specified dimensions, populating the data with the container
     *
     * If the container has a member function dim(), the second parameter may be omitted, and the dimensions of the container will be used.
     *
     * @tparam C The container type.
     * @param container The container containing pixel data.
     * @param dim The dimensions of the image in the container, may be omitted if the container has a member function dim().
     */
    template <typename C> requires (!std::derived_from<C, std::istream>)
    Grey_tif(C const& container, std::array<long,2> const& dim = {-1, -1}) noexcept : Grey_tif() {
        push_back(container, dim);
    }
    
    /**
     * @brief Constructor for Grey_tif that populates it from a TIFF input stream.
     *
     * @param is The input stream containing TIFF data.
     */
    Grey_tif(std::istream& is) : Grey_tif() {
        is.seekg(0, std::ios::end);
        std::streampos fileSize = is.tellg();
        is.seekg(0, std::ios::beg);
        d_tif.resize(static_cast<std::size_t>(fileSize));
        if (!is.read(reinterpret_cast<char*>(d_tif.data()), fileSize) ||
            (d_tif[0] != std::byte('I') && d_tif[0] != std::byte('M')) ||
            d_tif[0] != d_tif[1] || std::byte(42) != (d_tif[0] == std::byte('I') ? d_tif[2] : d_tif[3]) )
            is.setstate(std::ios::failbit);
        else
            f_scan_images();
    }
    
    /**
     * @brief swapping the contents of two TIFF stacks
     *
     * @tparam Tother The type of the Grey_tif object to be swapped.
     * @param other The Grey_tif object to be swapped.
     */
    template <typename Tother>
    void swap(Grey_tif<Tother>&& other) {
        d_last_ifd_offset = other.d_last_ifd_offset = 4;
         std::swap(d_tif, other.d_tif);
        f_scan_images();
        other.f_scan_images();
        if constexpr(!std::is_same_v<std::byte, T>)
            f_regularize();
        if constexpr(!std::is_same_v<std::byte, Tother>)
            other.f_regularize();
    }
    
    /**
     * @brief swapping the contents of two TIFF stacks
     *
     * Swaps the contents of this Grey_tif with that of another. If required, the data are regularized, so that they conform to the pixel type of the Grey_tif objects.
     *
     * @tparam Tother The type of the Grey_tif object to be swapped.
     * @param other The Grey_tif object to be swapped.
     */
    template <typename Tother>
    void swap(Grey_tif<Tother>& other) { swap(std::move(other)); }
    
    /**
     * @brief Read/write accessor to images from a single or multi-image Grey_tif object.
     *
     * @param i The index of the image in the stack.
     * @return A reference to the requested image.
     */
    auto const& image(std::size_t const i = 0) const { return d_img.at(i); }
    
    /**
     * @brief Read accessor to const images from a single or multi-image Grey_tif object.
     *
     * @param i The index of the image in the stack.
     * @return A reference to the requested image.
     */
    auto const& const_image(int const i = 0) const { return d_img_const.at(i); }
    
    /**
     * @brief Read/write accessor to images from a single or multi-image raw Grey_tif<std::byte> object.
     *
     * Read/write access by index. If the pixel type T0 is incorrect, the image has a zero size.
     *
     * @tparam T0 The desired pixel data type.
     * @param i The index of the image in the stack.
     * @return Image with the specified pixel type.
     */
    template <typename T0> requires std::is_same_v<T, std::byte>
    auto image(int const i) const { return static_cast<Grey_tif_image<T0>>(d_img.at(i)); }
    
    /**
     * @brief Accessor for retrieving a const image from a raw Grey_tif<std::byte> object, of which the pixels cannot be edited.
     *
     * Read access by index. If the pixel type T0 is incorrect, the image has a zero size.
     *
     * @tparam T0 The desired pixel data type.
     * @param i The index of the image in the stack.
     * @return Image with the specified pixel type.
     */
    template <typename T0> requires std::is_same_v<T, std::byte>
    auto  const_image(int const i) const { return Grey_tif_image<T0>(d_img_const.at(i)); }
    
    /**
     * @brief Get the number of images in the TIFF stack.
     *
     * @return The number of images in the TIFF stack.
     */
    std::size_t image_stack_size() const noexcept {return d_img.size();}
    
    /**
     * @brief Get the number of bytes of the TIFF data.
     *
     * @return The number of images bytes of the TIFF data.
     */
    std::size_t raw_data_size() const noexcept {return d_tif.size();}
    
    /**
     * @brief Write the TIFF data to an output stream.
     *
     * @param os The output stream where TIFF data will be written.
     */
    void write(std::ostream& os) const { os.write(reinterpret_cast<char const*>(d_tif.data()), d_tif.size());}
    
    /**
     * @brief Appends a new image from an STL container or a Grey_tif object to the end of the image stack.
     *
     * For a Grey_tif<std::byte> object (encodes raw TIFF data) that is appended, the pixel values in the input container must be  either
     * signed or unsigned 8-, 16-, 32-bit integrals,  float, or double.
     *
     * @tparam C The type of the container.
     * @param container The image container to push into the Grey_tif object.
     * @param dim The dimension of the image as a 2D array of long, can be omitted if the container has a member function dim().
     */
    template <typename C> requires (requires (C const& c) {std::begin(c), std::end(c), std::size(c);} || std::is_same_v<C, Grey_tif<std::byte>>)
    void push_back(C const& container, std::array<long,2> dim = {-1,-1}) {
        if constexpr (is_Grey_tif<C>::value) {
            for (int i = 0; i < container.image_stack_size(); ++i) {
                if constexpr (!std::is_same_v<std::byte, std::remove_const_t<typename C::value_type>>)
                    push_back(container.d_img[i]);
                else {
                    POD_type_traits const& img_type = container.image(i).type();
                    if      (img_type.is<std::int8_t>())   push_back(Grey_tif_image<std::int8_t>(container.d_img[i]));
                    else if (img_type.is<std::uint8_t>())  push_back(Grey_tif_image<std::uint8_t>(container.d_img[i]));
                    else if (img_type.is<std::int16_t>())  push_back(Grey_tif_image<std::int16_t>(container.d_img[i]));
                    else if (img_type.is<std::uint16_t>()) push_back(Grey_tif_image<std::uint16_t>(container.d_img[i]));
                    else if (img_type.is<std::int32_t>())  push_back(Grey_tif_image<std::int32_t>(container.d_img[i]));
                    else if (img_type.is<std::uint32_t>()) push_back(Grey_tif_image<std::uint32_t>(container.d_img[i]));
                    else if (img_type.is<float>())         push_back(Grey_tif_image<float>(container.d_img[i]));
                    else if (img_type.is<double>())        push_back(Grey_tif_image<double>(container.d_img[i]));
                }
            }
        }
        else {
            if constexpr (requires(C const& c) { c.dim().size();})
                if (dim[0] == -1 && dim[1] == -1) {
                    assert(container.dim().size() == 2); // Grey_tif.pushback(container) only accepts 2D containers
                    dim[0] = container.dim()[0];
                    dim[1] = container.dim()[1];
                }
            using CT = typename C::value_type;
            static_assert ((std::is_arithmetic_v<CT> && sizeof(CT) <= 8 && !std::is_same_v<CT, bool> && !(sizeof(CT) == 8 && std::is_integral_v<CT>)) ||
                           std::is_same_v<float, CT> || std::is_same_v<double, CT>);
            assert(container.size() == dim[0] * dim[1]);
            assert((d_tif.size() + dim[0] * dim[1] + 6 * 12 + 4) < std::numeric_limits<uint32_t>::max());
            std::uint32_t index = static_cast<uint32_t>(d_tif.size());
            std::uint32_t data_start = index;
            if constexpr (std::is_same_v<T, std::byte>) {
                d_tif.resize(d_tif.size() + dim[0] * dim[1] * sizeof(CT) + 7 * 12 + 6);
                for (auto const val : container) {
                    reinterpret_cast<CT&>(d_tif[index]) = static_cast<CT>(val);
                    index += sizeof(CT);
                }
            }
            else {
                d_tif.resize(d_tif.size() + dim[0] * dim[1] * sizeof(T) + 7 * 12 + 6);
                for (auto const val : container) {
                    reinterpret_cast<T&>(d_tif[index]) = static_cast<T>(val);
                    index += sizeof(T);
                }
            }
            if ((d_tif.size() & 1) == 1) {
                d_tif.push_back(std::byte(0));
                ++index;
            }
            reinterpret_cast<std::uint32_t&>(d_tif[d_last_ifd_offset]) = index;
            reinterpret_cast<std::uint16_t&>(d_tif[index]) = 7;
            index += 2;
            f_set_ifd(index, 0x0100, 3, static_cast<uint32_t>(dim[1]));
            f_set_ifd(index, 0x0101, 3, static_cast<uint32_t>(dim[0]));
            f_set_ifd(index, 0x0102, 3, 8 * (std::is_same_v<std::byte, T> ? sizeof(CT) : sizeof(T)));
            f_set_ifd(index, 0x0103, 3, 1);
            f_set_ifd(index, 0x0106, 3, 1);
            f_set_ifd(index, 0x0111, 4, data_start);
            if constexpr (std::is_same_v<std::byte, T>) {
                if (std::is_unsigned_v<CT> )
                    f_set_ifd(index, 0x0153, 3, 1);
                else if (std::is_integral_v<CT>)
                    f_set_ifd(index, 0x0153, 3, 2);
                else if (std::is_floating_point_v<CT>)
                    f_set_ifd(index, 0x0153, 3, 3);
            }
            else {
                if (std::is_unsigned_v<T> )
                    f_set_ifd(index, 0x0153, 3, 1);
                else if (std::is_integral_v<T>)
                    f_set_ifd(index, 0x0153, 3, 2);
                else if (std::is_floating_point_v<T>)
                    f_set_ifd(index, 0x0153, 3, 3);
            }
            d_tif[index] = d_tif[index + 1] = d_tif[index + 2] = d_tif[index + 3] = std::byte(0);
            d_last_ifd_offset = static_cast<uint32_t>(index);
            f_scan_images();
        }
    }
    
    /**
     * @brief Appends an empty  image to the end of the image stack.
     *
     * @tparam C The pixel type of the empty image.
     * @param dim The dimension of the image as a 2D array of long.
     */
    template <typename C = T> requires (std::is_same_v<T,C> || (std::is_same_v<T, std::byte> &&
                                                            ((std::is_integral_v<C> && sizeof(T) <= 4 && !std::is_same_v<C, bool>) ||
                                                             (std::is_floating_point_v<C> && sizeof(C) <= 8))))
    void push_back(std::array<long,2> const dim) {
        f_push_back(dim, POD_type_traits{.size = sizeof(C), .is_signed = std::is_signed_v<C>, .is_integral = std::is_integral_v<C>});
    }
    
    /**
     * @brief Wipes all images, resetting to an empty Grey_tif object
     */
    Grey_tif& clear() {
        Grey_tif tmp;
        this->swap(tmp);
        this->d_image_type = {.size = sizeof(T), .is_signed = std::is_signed_v<T>, .is_integral = std::is_integral_v<T>};
        this->d_dim = {0, 0};
        std::span<T const>::operator=({});
        return *this;
    }
    
private:
    std::uint32_t d_last_ifd_offset = 0; // location of the last tif tag
    std::vector<std::byte> d_tif; // raw tiff data as std::byte
    std::vector<Grey_tif_image<T>> d_img; // vector of images that retrieve their pixel values from d_tif
    std::vector<Grey_tif_image<T const>> d_img_const; // vector of images that retrieve their pixel values from d_tif
    
    template <typename T0> struct is_Grey_tif :  std::false_type {};
    template <typename T0> struct is_Grey_tif<Grey_tif<T0>> :  std::true_type {};
    
    bool f_all_same_type(POD_type_traits const& check_type) const noexcept {
        for (auto it = d_img.begin(); it != d_img.end(); ++it)
            if (it->type() != check_type)
                return false;
        return true;
    }
    
    // Push_back an all-zero image of the specified type
    void f_push_back(std::array<long,2> const& dim, POD_type_traits const& new_type) {
        assert((d_tif.size() + static_cast<std::size_t>(dim[0] * dim[1]) + 6 * 12 + 4) < std::numeric_limits<uint32_t>::max());
        assert((this->type().template is<T>() || std::is_same_v<T, std::byte>));
        std::uint32_t index = static_cast<uint32_t>(d_tif.size());
        std::uint32_t data_start = index;
        d_tif.resize(d_tif.size() + static_cast<std::size_t>(dim[0] * dim[1]) * new_type.size + 7 * 12 + 6);
        index += static_cast<std::size_t>(dim[0] * dim[1]) * new_type.size;
        if ((d_tif.size() & 1) == 1) {
            d_tif.push_back(std::byte(0));
            ++index;
        }
        reinterpret_cast<std::uint32_t&>(d_tif[d_last_ifd_offset]) = index;
        reinterpret_cast<std::uint16_t&>(d_tif[index]) = 7;
        index += 2;
        f_set_ifd(index, 0x0100, 3, static_cast<uint32_t>(dim[0]));
        f_set_ifd(index, 0x0101, 3, static_cast<uint32_t>(dim[1]));
        f_set_ifd(index, 0x0102, 3, 8 * static_cast<uint32_t>(new_type.size));
        f_set_ifd(index, 0x0103, 3, 1);
        f_set_ifd(index, 0x0106, 3, 1);
        f_set_ifd(index, 0x0111, 4, data_start);
        f_set_ifd(index, 0x0153, 3, new_type.is_integral ? (!new_type.is_signed ? 1 : 2) : 3);
        d_tif[index] = d_tif[index + 1] = d_tif[index + 2] = d_tif[index + 3] = std::byte(0);
        d_last_ifd_offset = static_cast<uint32_t>(index);
        f_scan_images();
    }
    
    Grey_tif& f_regularize() noexcept {
        static_assert(!std::is_same_v<T, std::byte>);
        POD_type_traits new_type = {.size = sizeof(T), .is_signed = std::is_signed_v<T>, .is_integral = std::is_integral_v<T> };
        if (f_all_same_type(new_type))
            return *this;
        else {
            bool same_size = this->type().size == new_type.size;
            for (auto it = d_img.begin() + 1; it != d_img.end() && same_size == true; ++it)
                same_size = it->type().size == new_type.size;
            if (same_size) {
                for (auto& img : d_img) {
                    if (!new_type.is_integral && img.type().is_integral && img.type().is_signed)
                        for (auto& pixel : img)
                            reinterpret_cast<float&>(pixel) = static_cast<float>(reinterpret_cast<int&>(pixel));
                    else if (!new_type.is_integral && img.type().is_integral && !img.type().is_signed)
                        for (auto& pixel : img)
                            reinterpret_cast<float&>(pixel) = static_cast<float>(reinterpret_cast<unsigned int&>(pixel));
                    else if (new_type.is_integral && new_type.is_signed && !img.type().is_integral)
                        for (auto& pixel : img)
                            reinterpret_cast<std::int32_t&>(pixel) = static_cast<int>(reinterpret_cast<float&>(pixel));
                    else if (new_type.is_integral && !new_type.is_signed && !img.type().is_integral)
                        for (auto& pixel : img)
                            reinterpret_cast<std::uint32_t&>(pixel) = static_cast<unsigned>(reinterpret_cast<float&>(pixel));
                    img = Grey_tif_image(new_type, img.dim(), img);
                }
            }
            else {
                Grey_tif new_tif;
                for (std::size_t i=0; i != image_stack_size(); ++i) {
                    new_tif.f_push_back(image(i).dim(), new_type);
                    POD_type_traits const& old_type = image(i).type();
                    std::size_t size = static_cast<std::size_t>(image(i).dim()[0] * image(i).dim()[1]);
                    auto new_begin = new_tif.image(i).begin();
                    if      (old_type.is<std::int8_t>())   std::copy_n(reinterpret_cast<std::int8_t*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<std::uint8_t>())  std::copy_n(reinterpret_cast<std::uint8_t*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<std::int16_t>())  std::copy_n(reinterpret_cast<std::int16_t*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<std::uint16_t>())  std::copy_n(reinterpret_cast<std::uint16_t*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<std::int32_t>())  std::copy_n(reinterpret_cast<std::int32_t*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<std::uint32_t>())  std::copy_n(reinterpret_cast<std::uint32_t*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<float>())  std::copy_n(reinterpret_cast<float*>(&d_img[i][0])  , size, new_begin);
                    else if (old_type.is<double>())  std::copy_n(reinterpret_cast<double*>(&d_img[i][0])  , size, new_begin);
                }
                std::swap(new_tif, *this);
            }
            return *this;
        }
    }
    
    void f_scan_images() {
        d_img.clear();
        d_img_const.clear();
        this->d_dim = {0,0};
        bool native = d_tif[0] == std::byte('I') && std::endian::native == std::endian::little;
        if (!native)
            d_tif[0] = d_tif[1] = (d_tif[0] == std::byte('M')) ? std::byte('I'): std::byte('M');
        reinterpret_cast<std::uint16_t&>(d_tif[2]) = 42;
        std::uint32_t index = reinterpret_cast<std::uint32_t&>(d_tif[4]);
        if (!native) {
            index = (index << 24) | ((index & 0xff00) << 8) | ((index >> 8) & 0xff00) | (index >> 24);
            reinterpret_cast<std::uint32_t&>(d_tif[4]) = index;
        }
        if (native)
            while (index != 0) {
                f_make_Image<true>(index);
                index = reinterpret_cast<std::uint32_t&>(d_tif[d_last_ifd_offset]);
            }
        else
            while (index != 0) {
                f_make_Image<false>(index);
                index = reinterpret_cast<std::uint32_t&>(d_tif[d_last_ifd_offset]);
            }
        if (image_stack_size() != 0) {
            if constexpr(!std::is_same_v<T, std::byte>)
                std::span<T const>::operator=(d_img_const[0]);
            else
                this->d_data = d_img_const[0].d_data;
            this->d_dim = d_img[0].dim();
            this->d_image_type = d_img[0].type();
        }
        if constexpr (!std::is_same_v<T, std::byte>)
            f_regularize();
    }
    
    template <bool NATIVE = true>
    void f_make_Image(std::uint32_t& index) {
        bool compatible_tif = true;
        std::array<long,2> dim = {0,0};
        std::size_t bits_per_pixel = 0;
        std::byte* cursor = d_tif.data() + index;
        int const tag_count = f_int16<NATIVE>(cursor);
//        std::int32_t rows_per_strip;
        std::vector<std::int32_t> strip_offsets(1);
        std::vector<std::int32_t> strip_byte_counts(1);
        bool signed_pixels = false;
        bool int_pixels = true;
        for (int i = 0; i != tag_count; ++i) {
            uint16_t const tag = f_int16<NATIVE>(cursor);
            uint16_t const type = f_int16<NATIVE>(cursor);
            uint32_t const count = f_int32<NATIVE>(cursor);
            uint32_t val(0);
            double double_val(0);
            if ((type == 1) || (type == 2) || (type == 6) || (type == 7)) {
                val = f_int8(cursor);
                cursor += 3;
            }
            else if ((type == 3) || (type == 8)) {
                val = f_int16<NATIVE>(cursor);
                cursor += 2;
            }
            else if ((type == 4) || (type == 9))
                val = f_int32<NATIVE>(cursor);
            else if (type == 5 || type == 10) {
                std::byte* p = d_tif.data() + f_int32<NATIVE>(cursor);
                double_val = f_int32<NATIVE>(p);
                double_val /= f_int32<NATIVE>(p);
            }
            else if (type == 11) {
                val = f_int32<NATIVE>(cursor);
                double_val = reinterpret_cast<double&>(val);
                val = 0;
            }
            else if (type == 12) {
                std::byte* p = d_tif.data() + f_int32<NATIVE>(cursor);
                std::uint64_t tmp = f_int64<NATIVE>(p);
                double_val = reinterpret_cast<double&>(tmp);
            }
            if (tag == 0x0100) dim[0] = val;
            else if (tag == 0x0101) dim[1] = val;
            else if (tag == 0x0102 ) {
                if ((val == 8 || val == 16 || val == 32 || val == 64))
                    bits_per_pixel = val;
                else {
                    std::cerr << "Warning: Grey_tif can only read greyscale tiff files with 8-, 16-, 32-, or 64-bit pixels" << std::endl;
                    compatible_tif = false;
                }
            }
            else if (tag == 0x0103 && val != 1) {
                std::cerr << "Warning: Grey_tif cannot read compressed tiff files"<< std::endl;
                compatible_tif = false;
            }
            else if (tag == 0x0106 && val > 1) {
                std::cerr << "Warning: Grey_tif cannot read colour tiff files"<< std::endl;
                compatible_tif = false;
            }
            else if ((tag == 0x0107 || tag == 0x0108 || tag == 0x0109 || tag == 0x010A) && val != 1) {
                std::cerr << "Warning: Grey_tif cannot read black & white tiff files"<< std::endl;
                compatible_tif = false;
            }
            else if (tag == 0x0111) {
                if (count == 1) strip_offsets[0] = static_cast<std::int32_t>(val);
                else {
                    strip_offsets.resize(count);
                    std::byte* p = d_tif.data() + val;
                    for (std::size_t j = 0; j != static_cast<std::size_t>(count); ++j)
                        strip_offsets[j] = static_cast<std::int32_t>(f_int32<NATIVE>(p));
                }
            }
            else if (tag == 0x0115 && val != 1) {
                std::cerr << "Warning: Grey_tif cannot read RGB colour tiff files"<< std::endl;
                compatible_tif = false;
            }
//            else if (tag == 0x0116) rows_per_strip = static_cast<std::int32_t>(val);
            else if (tag == 0x0117) {
                if (count == 1) strip_byte_counts[0] = static_cast<std::int32_t>(val);
                else {
                    strip_byte_counts.resize(count);
                    std::byte* p = d_tif.data() + val;
                    for (std::size_t j=0; j != count; ++j)
                        strip_byte_counts[j] = static_cast<std::int32_t>(f_int32<NATIVE>(p));
                }
            }
            else if (tag == 0x0153) {
                if (val != 1) signed_pixels = true;
                if (val == 3) int_pixels = false;
            }
        }
        for (std::size_t i=0; i != strip_offsets.size() - 1; ++i)
            if (strip_byte_counts[i] != strip_offsets[i + 1] - strip_offsets[i]) {
                std::cerr << "Warning: Grey_tif cannot read tiff files with non-consecutive strips" << std::endl;
                std::cerr << "         most likely the tiff file is corrupted" << std::endl;
                compatible_tif = false;
                break;
            }
        if (!compatible_tif) 
            throw std::runtime_error("Incompatible TIFF file\n");
        d_last_ifd_offset = static_cast<std::uint32_t>(cursor - d_tif.data());
        index = f_int32<NATIVE>(cursor);
        if constexpr (!NATIVE) {
            std::byte* begin = d_tif.data() + strip_offsets[0];
            std::byte* end = begin + static_cast<std::size_t>(dim[0] * dim[1]) * bits_per_pixel / 8;
            if (bits_per_pixel == 16)
                while (begin != end) f_int16<false>(begin);
            else if (bits_per_pixel == 32)
                while (begin != end) f_int32<false>(begin);
            else if (bits_per_pixel == 64)
                while (begin != end) f_int64<false>(begin);
        }
        POD_type_traits image_type = { .size = bits_per_pixel / 8, .is_signed = signed_pixels, .is_integral = int_pixels };
        d_img.push_back(Grey_tif_image<T>(image_type, dim,
                                          std::span<T>(reinterpret_cast<T*>(&d_tif[static_cast<std::size_t>(strip_offsets[0])]),
                                                       static_cast<std::size_t>(dim[0] * dim[1]))));
        d_img_const.push_back(Grey_tif_image<T const>(image_type, dim,
                                                      std::span<T const>(reinterpret_cast<T const*>(&d_tif[static_cast<std::size_t>(strip_offsets[0])]),
                                                                         static_cast<std::size_t>(dim[0] * dim[1]))));
    };
    
    void f_set_ifd(std::uint32_t& index, uint16_t tag, uint16_t type, uint32_t val) {
        reinterpret_cast<uint16_t&>(d_tif[index]) = tag;
        reinterpret_cast<uint16_t&>(d_tif[index + 2]) = type;
        reinterpret_cast<uint32_t&>(d_tif[index + 4]) = 1;
        if (type == 1)
            reinterpret_cast<uint8_t&>(d_tif[index + 8]) = static_cast<uint8_t>(val);
        else if (type == 3)
            reinterpret_cast<uint16_t&>(d_tif[index + 8]) = static_cast<uint16_t>(val);
        else if (type == 4)
            reinterpret_cast<uint32_t&>(d_tif[index + 8]) = val;
        index += 12;
    };
    
    std::uint8_t f_int8(std::byte* &cursor)  {
        return reinterpret_cast<std::uint8_t&>(*cursor++);
    }
    
    template <bool NATIVE>
    constexpr std::uint16_t f_int16(std::byte* &cursor) {
        std::uint16_t r;
        std::memcpy(&r, cursor, sizeof(std::uint16_t));
        cursor += 2;
        return NATIVE ? r : r = static_cast<uint16_t>(r << 8) | static_cast<uint16_t>(r >> 8);
    }
    
    template <bool NATIVE>
    constexpr std::uint32_t f_int32(std::byte* &cursor) const {
        std::uint32_t r;
        std::memcpy(&r, cursor, sizeof(std::uint32_t));
        cursor += 4;
        return NATIVE ? r : r = (r << 24) | ((r & 0x00FF0000) >> 8) | ((r & 0x0000FF00) << 8) | (r >> 24);
    }
    
    template <bool NATIVE>
    constexpr std::uint64_t f_int64(std::byte* &cursor) {
        std::uint64_t r;
        std::memcpy(&r, cursor, sizeof(std::uint64_t));
        cursor += 8;
        return NATIVE ? r : r = ((r << 56) | ((r & 0xff00) << 40) | ((r & 0xff0000) << 24) | ((r & 0xff000000) << 8) |
                             ((r >> 8) & 0xff000000) | ((r >> 24) & 0xff0000) | ((r >> 40) & 0xff00) | (r >> 56));
    }
};

// Constructor deduction guide for Grey_tif
template <typename C> Grey_tif(C const& container, std::array<long, 2> const& dim) -> Grey_tif<typename C::value_type>;

} // namespace jpa
#endif /* Grey_tif_h */
