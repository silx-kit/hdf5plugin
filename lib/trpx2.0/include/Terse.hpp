//
//  Terse.hpp
//  Terse
//
//  Created by Jan Pieter Abrahams on 30/04/2019.
//  Copyright © 2019 Jan Pieter Abrahams. All rights reserved.
//

#ifndef Terse_h
#define Terse_h

#include <fstream>
#include <istream>
#include <vector>
#include <charconv>
#include <cassert>
#include <numeric>
#include <span>
#include <future>
#include <algorithm>
#include <type_traits>
#include <optional>
#include <variant>
#include <cmath>
#include "Bitqueue.hpp"
#include "Unique_array.hpp"
#include "XML_element.hpp"

// Terse<C> allows efficient and fast compression of integral diffraction data and other integral greyscale
// data into a Terse object that can be decoded by the member function prolix(). The
// prolix(iterator) member function decompresses the data starting at the location defined by 'iterator'
// (which can also be a pointer). A Terse object is constructed by supplying it with uncompressed data or a
// stream that contains compressed Terse data.
//
// A Terse object may contain compressed data of multiple frames, and data of a particular frame can be
// extracted by indexing. All frames must have the same size and dimensions, and must all be signed or unsigned.
//
// The template parameter C is either void or Concurrent. If C is Concurrent, decompressing the multiple frames
// into a std::vector is multi-threaded, and compressing is multithreaded if the data to be compressed are
// passed as an rvalue container (e.g. Terse<Concurrent> terse(std::move(data_vector));). This empties the
// rvalue container, thus optimizing memory use.
//
// A Terse object can be unpacked into any arithmetic type T, including float and double. Unpacking into
// values of a type with fewer bits than the original data is not allowed. Compressing as unsigned yields a tighter
// compression, because the sign bit does not need to be encoded.
//
// A Terse object can be written or appended to any stream. The resulting file is independent of the endian-nes
// of the machine: both big- and small-endian machines produce identical files, making data transfer optimally
// transparent.
//
// Terse data in a file are immediately preceded by a small header, which is encoded in standard XML as follows:
// <Terse prolix_bits="n" signed="s" block="b" number_of_values="v" number_of_frames="f" memory_size="m"
//  [dimensions="d [...]"] [metadata_string_sizes="ms"] [memory_sizes_of_frames="mf"]/>
//   - "n" is the number of bits required for the most extreme value in the Terse data, representing the bit depth of
//         the original, uncompressed data.
//   - "s" is "0" for unsigned data, "1" for signed data.
//   - "b" is the block size of the stretches of data values that are encoded (by default 12 values).
//   - "v" is the number of elements of a single frame of a stack (it is the product of the dimensions, if these
//         are provided).
//   - "f" is the number of frames encoded in the file.
//   - "m" is the total memory size of all frames, excluding the header and metadata.
//   - "d [...]" is optional and encodes the dimensions of a single frame. Frames can have any number of dimensions.
//   - "ms" is optional and encodes the sizes of metadata strings associated with each frame.
//   - "mf" is optional and encodes the memory sizes of individual frames in the stack.
// Here is an example:
//      <Terse prolix_bits="12" signed="0" block="12" number_of_values="262144" number_of_frames="2" memory_size="91388"
//      dimensions="512 512" memory_sizes_of_frames="45694 45694" metadata_string_sizes="10 15"/>
//
// The default algorithm is a run-length encoding type. Each data block (by default 12 integral values in the
// constructor, but this can be changed) is preceded by one or more data block header bits. The values in the
// data block are stripped of their most significant bits, provided they are all zero (for unsigned values),
// or either all zero or all one (for signed values). In the latter case, the sign bit is maintained. So, for a
// block size of 3 with values 3, 4, 2, the encoded bits would be: 011 (denoting 3) 100 (denoting 4) 010
// (denoting 2). Hence 011100010 would be pushed into the Terse object. In case of signed values -3, 4, 2, the
// encoded bits would be 1011 (denoting -3) 0100 (denoting +4) 0010 (denoting +2), resulting in a data block
// 101101000010. So if the values that need to be encoded are all positive or zero, they should be encoded as
// unsigned for optimal compression: it saves 1 bit per encoded value.
//
// The header bits define how the values are encoded. They have the following following structure:
// bit 1:    If the first bit of the block header is set, then there are no more bits in the block header,
//           and the parameters of the previous block header are used.
// bit 2-4:  The first header bit is 0. The three bits 2 to 4 define how many bits are used per value of the
//           encoded block. If bits 2 to 4 are all set, 7 or more bits per value are required and the
//           header is expanded by a further 2 bits.
// bit 5-6:  The first 4 header bits are 0111. The number encoded bits 5 and 6 is added to 7 and this defines
//           how many bits are used to encode the block. So if bits 5 & 6 are 00 then 7 bits are used, 01 means
//           8 bits, 10 means 9 bits and 11 means at least 10 bits. If bits 5 & 6 are both set, the header is
//           expanded by another 6 bits.
// bit 7-12: The first 6 header bits are 011111. The number encoded by bits 7 to 12 is added to decimal 10 and
//           this defines how many bits are used to encode the block. So if bits 7 to 12 are 000000, then the
//           number of bits per value in the data block is decimal 10. If bits 7 to 12 are 110110, then the
//           number of bits per value in the data block is 10 + 54 = 64.
// The default compression mode is compatible with ealier versions of Terse and available through the flag Terse_mode::Signed.
//
// For unsigned data that contains overloads (all bits set to 1), a separate encoding scheme is available through
// the flag Terse_mode::Unsigned. In this scheme, the algorithm increments all values in the block by 1 prior to encoding.
// This optimization saves space when many overloads are present. However, earlier versions of Terse will not be able
// to read this data.
//
// An additional algorithm improves compression for very weak data available through the flag Terse_mode::Small_unsigned.
// Blocks with a maximum value of 2, 4, 5, or 6 are encoded using an alternative approach, where values are represented
// in a number base system with radices 3, 5, 6, and 7, respectively. This method reduces the number of bits required
// for encoding these blocks. Furthermore, the algorithm leverages patterns between consecutive blocks. If all values in
// the current block are smaller than 7, it checks whether the block differs by 1 from the previous block. For blocks with
// values greater than 6, it checks if the significant number of bits differs by 1 from the previous block. In these cases,
// the new block size or precision can be inferred from the previous block, further reducing the overhead. Additionally,
// blocks with all zero values are encoded using a single bit, maximizing efficiency in cases with sparse data. Earlier
// versions of Terse will not be able to read this data.
//
// Constructors:
//  Terse<C>(std::ifstream& istream)
//      Reads in a Terse object that has been written to a file by Terse<C>::write(...).
//  Terse<C>(container_type&& data, Terse_mode const mode = Terse_mode::Signed)
//      Creates a Terse object from data (which can be a std::vector, Field, etc.). Only containers of
//      integral types are allowed. If the container has a member function dim(), that will set the dimensions
//      of the Terse object. Otherwise the dimensions can be set once using the dim(vector const&) member function.
//      If the the 'data' parameter is an rvalue and the Terse template parameter C is Concurrent, compression is
//      branched to a different thread and proceeds concurrently. In this case, the 'data' container is emptied.
//  Terse<C>(iterator begin, std::size_t size, Terse_mode const mode = Terse_mode::Signed)
//      Creates a Terse object given a starting iterator or pointer and the number of elements that need to be
//      encoded.
//
// Member functions:
//  void insert(std::size_t const pos, Iterator const data, size_t const size, Terse_mode const mode = Terse_mode::Signed) noexcept
//      Adds another frame to the Terse object at the position defined by 'pos'. The new frame is defined by
//      its begin iterator and size.
//  void insert(std::size_t const pos, C&& data, Terse_mode const mode = Terse_mode::Signed) noexcept
//      Inserts a frame into the Terse object at the position defined by 'pos'. The new frame is defined by a reference
//      to a container. The size and dimensions of the input container must be the same as that of the first frame that
//      was used for creating the Terse object, unless the Terse object is empty, in which case the inserted container
//      determines the size and dimension of subsequent frames that are added.
//      If the input container is passed as an r-value, it will be consumed, and will be empty after the call.
//      If the input container is passed as an r-value, and the include file Concurrent.hpp is included, compression
//      will be performed concurrently in the background.
//  void push_back(Iterator const begin, size_t const size, Terse_mode const mode = Terse_mode::Signed)
//      Adds another frame to the Terse object, given a starting iterator or pointer and the number of elements
//      that need to be encoded. The 'size' must be identical to that of existing frames.
//  void push_back(container_type&& container, Terse_mode const mode = Terse_mode::Signed)
//      Adds another frame to the Terse object. The new frame is defined by its begin iterator and size, or by a
//      reference to a container. The size must be the same as that of the frame used to create the Terse object.
//      If the container has a member function dim(), that must return the same dimension as provided for the first
//      frame.
//      If the the 'data' parameter is an rvalue and the Terse template parameter C is Concurrent, compression is
//      branched to a different thread and proceeds concurrently. In this case, the 'data' container is emptied.
//  void erase(std::size_t pos) noexcept
//      Removes the frame with index 'pos' from the Terse object. Also waits until concurrent compression has finished,
//      and releases unused storage to the heap.
//  Terse at(std::size_t pos) noexcept
//      Returns the frame with index 'pos' as a Terse object.
//  void prolix(iterator begin, std::size_t const pos = 0)
//      Unpacks the Terse frame with index 'pos', storing it from the location defined by 'begin'.
//      Terse integral signed data cannot be unpacked into integral unsigned data. Terse data cannot be decompressed
//      into elements that have fewer bits than bits_per_val(), but can be decompressed into larger values. Terse data
//      can always be unpacked into signed integral, double and float data and will have the correct sign (with one
//      exception: an unsigned overflowed - all 1's - value will be unpacked as -1 signed value. As all other values are
//      positive in this case).
//  void prolix(container_type& container, std::size_t const pos = 0)
//      Unpacks the Terse frame with index 'pos' and stores it in the provided container. Also checks the container is
//      large enough.
//  std::size_t size() const noexcept
//      Returns the number of encoded elements.
//  std::size_t const number_of_frames() const
//      Returns the number of frames stored in the Terse object
//  std::vector<std::size_t> const& dim() const noexcept
//      Returns the dimensions of the frames (all frames must have the same dimensions).
//  void dim(std::vector<std::size_t> const& dim) noexcept
//      Sets the dimensions of the Terse frames. Since all frames must have the same dimensions, they can be set only once.
//  bool is_signed() const noexcept
//      Returns true if the encoded data are signed, false if unsigned. Signed data cannot be decompressed into
//      unsigned data.
//  unsigned bits_per_val() const noexcept
//      Returns the maximum number of bits per element that can be expected. So for uncompressed uint_16 type
//      data, bits_per_val() returns 16. Terse data cannot be decompressed into a container type with elements
//      that are smaller in bits than bits_per_val().
//  std::size_t terse_size() noexcept
//      Returns the number of bytes used for encoding the Terse data.
//  void write(std::ostream& ostream)
//      Writes Terse data to 'ostream'. The Terse data are preceded by an XML element containing the parameters
//      that are required for constructing a Terse object from the stream. Data are written as a byte stream
//      and are therefore independent of endian-ness. A small-endian memory lay-out produces the a Terse file
//      that is identical to a big-endian machine.
//  void shrink_to_fit() noexcept
//      Releases unused buffer storage to heap memory. This increases available heap memory when Terse objects were constructed
//      from uncompressed data sources held in memory. If compression is performed concurrently, also waits for all compression
//      processes to finish. It has no effect when Terse object are read from a stream.
//  std::string const& metadata(std::size_t frame = 0) const noexcept
//      Returns the metadata that are associated with the specified frame.
//  void metadata(std::string data, std::size_t frame = 0) noexcept
//      Sets / overwrites any optional metadata that are associated with the specified frame. Metadata are not compressed.
//      Metadata will be written to, and read from a stream as part of the Terse object.
//
// Example:
//
//    std::vector<int> numbers(1000);                   // Uncompressed data location
//    std::iota(numbers.begin(), numbers.end(), -500);  // Fill with numbers -500, -499, ..., 499
//    Terse trpx(numbers);                        // Compress the data to less than 30% of memory
//    std::cout << "compression rate " << float(compressed.terse_size()) / (numbers.size() * sizeof(unsigned)) << std::endl;
//    std::ofstream outfile("junk.trpx");
//    compressed.write(outfile);                        // Write Terse data to disk
//    std::ifstream infile("junk.trpx");
//    Terse from_file(infile);                          // Read it back in again
//    std::vector<int> uncompressed(1000);
//    from_file.prolix(uncompressed.begin());           // Decompress the data...
//    for (int i=0; i != 5; ++i)
//    std::cout << uncompressed[i] << std::endl;
//    for (int i=995; i != 1000; ++i)
//    std::cout << uncompressed[i] << std::endl;
//
// Produces as output:
//
//    compression rate 0.29
//    -500
//    -499
//    -498
//    -497
//    -496
//    495
//    496
//    497
//    498
//    499


namespace jpa {
template <typename T>
concept Container = requires(T& c) {
    { c.begin() } -> std::input_iterator;
    { c.end() } -> std::sentinel_for<decltype(c.begin())>;
    { c.size() } -> std::convertible_to<std::size_t>;
//    { c.data() } -> std::convertible_to<typename std::remove_reference<decltype(*c.begin())>::type*>;
};

/**
 * @class Terse
 * @brief Terse<T> allows efficient and fast compression of integral diffraction data and other integral greyscale data.
 *
 * The Terse class is used for compression and decompression of integral diffraction data and other integral greyscale data.
 * It supports efficient compression and decompression, as well as data extraction and output to streams.
 *
 * A Terse object may contain compressed data of multiple frames, and data of a particular frame can be
 * extracted by indexing. All frames in a Terse object must have the same size and dimensions, and must all be signed or unsigned.
 *
 * A Terse object can be unpacked into any arithmetic type, including float and double. Unpacking into
 * values of type T with fewer bits than the original data results in truncation of overflowed data to
 * std::numeric_limits<T>::max() and to std::numeric_limits<T>::min() for underflowed signed types.
 * Unpacking signed data into unsigned values is not allowed. Compressing as unsigned yields a tighter compression.
 *
 * A Terse object can be written or appended to any stream. The Terse data in the file is independent of the endian-ness
 * of the machine: both big-endian and small-endian machines produce identical files, making data transfer optimally
 * transparent.
 *
 * Terse data in a file are immediately preceded by a small header, which is encoded in standard XML as follows:
 * <Terse prolix_bits="n" signed="s" block="b" number_of_values="v" number_of_frames="f" memory_size="m"
 *  [dimensions="d [...]"] [metadata_string_sizes="ms"] [memory_sizes_of_frames="mf"]/>
 *   - "n" is the number of bits required for the most extreme value in the Terse data, representing the bit depth of
 *         the original, uncompressed data.
 *   - "s" is "0" for unsigned data, "1" for signed data.
 *   - "b" is the block size of the stretches of data values that are encoded (by default 12 values).
 *   - "v" is the number of elements of a single frame of a stack (it is the product of the dimensions, if these are provided).
 *   - "f" is the number of frames encoded in the file.
 *   - "m" is the total memory size of all frames, excluding the header and metadata.
 *   - "d [...]" is optional and encodes the dimensions of a single frame. Frames can have any number of dimensions.
 *   - "ms" is optional and encodes the sizes of metadata strings associated with each frame.
 *   - "mf" is optional and encodes the memory sizes of individual frames in the stack.
 *
 * Here is an example of a Terse file with two frames of 512x512 pixels:
 * <pre>
 *       <Terse prolix_bits="12" signed="0" block="12" number_of_values="262144" number_of_frames="2" memory_size="91388" dimensions="512 512" memory_sizes_of_frames="45694 45694" metadata_string_sizes="10 15"/>
 * </pre>
 *
 * Example of usage:
 * \code{.cpp}
 *    std::vector<int> numbers(1000);                   // Uncompressed data location
 *    std::iota(numbers.begin(), numbers.end(), -500);  // Fill with numbers -500, -499, ..., 499
 *    Terse trpx(numbers);                              // Compress the data to less than 30% of memory
 *    std::cout << "compression rate " << float(compressed.terse_size()) / (numbers.size() * sizeof(unsigned)) << std::endl;
 *    std::ofstream outfile("junk.trpx");
 *    compressed.write(outfile);                        // Write Terse data to disk
 *    std::ifstream infile("junk.trpx");
 *    Terse from_file(infile);                          // Read it back in again
 *    std::vector<int> uncompressed(1000);
 *    from_file.prolix(uncompressed.begin());           // Decompress the data...
 *    for (int i = 0; i != 5; ++i)
 *        std::cout << uncompressed[i] << std::endl;
 *    for (int i = 995; i != 1000; ++i)
 *        std::cout << uncompressed[i] << std::endl;
 * \endcode
 *
 * Produces the following output:
 * <pre>
 *    compression rate 0.29
 *    -500
 *    -499
 *    -498
 *    -497
 *    -496
 *    495
 *    496
 *    497
 *    498
 *    499
 * </pre>
 */

class Concurrent;

namespace detail {
struct Terse_copyable {};

/**
 * @brief Empty base class that makes Terse<Concurrent> move-only.
 *
 * Terse<Concurrent> stores frames that are still being compressed as std::future objects, which cannot be
 * copied. Without this base, the implicitly declared copy constructor and copy assignment would not be deleted
 * (std::vector does not constrain its copy operations), so std::is_copy_constructible would report true and
 * code that relies on it (e.g. pybind11) would fail to compile when it tries to copy.
 */
struct Terse_move_only {
    Terse_move_only() = default;
    Terse_move_only(Terse_move_only const&) = delete;
    Terse_move_only& operator=(Terse_move_only const&) = delete;
    Terse_move_only(Terse_move_only&&) = default;
    Terse_move_only& operator=(Terse_move_only&&) = default;
};
} // end namespace detail

/**
 * @enum Terse_mode
 * @brief Defines the modes used for encoding data in a Terse object.
 *
 * The `Terse_mode` enum specifies whether specific optimizations are applied for (small) unsigned values.
 * It also allows backwards compatibility with earlier versions of Terse by selecting Signed.
 *
 * Enumerators:
 * - **Signed**: The fastest and most general compression mode: the sign bit is maintained for signed data,
 *   enabling proper decompression of signed values. Unsigned data is compressed without sign bit, allowing
 *   tighter compression. Data stored as Signed can be read by earlier versions of Terse.
 * - **Unsigned**: Data is treated as unsigned for compression; overloads (values with all bits set) are encoded
 *   efficiently. Unsigned compression is just as fast as the Signed compression. Earlier versions of Terse cannot
 *   read Unsigned data.
 * - **Small_unsigned**: Optimized encoding mode for small unsigned values, providing tighter compression
 *   for values that require fewer bits. However, it is a bit slower than Unsigned or Signed compression.  Earlier
 *   versions of Terse cannot read Small_unsigned data.
 *
 * Usage example:
 * \code{.cpp}
 * Terse_mode mode = Terse_mode::Signed; // Specify signed mode for data compression.
 * \endcode
 */
enum class Terse_mode {
    Unsigned,
    Small_unsigned,
    Signed,
    Default
};

template<typename CONCURRENT = void>
class Terse : private std::conditional_t<std::is_same_v<CONCURRENT, Concurrent>, detail::Terse_move_only, detail::Terse_copyable> {
    static_assert(std::is_same_v<CONCURRENT, void> || std::is_same_v<CONCURRENT, Concurrent>, "Terse is either Concurrent or not");
#ifndef Concurrent_h
    static_assert(std::is_same_v<CONCURRENT, void>, "Concurrent.hpp must be included before #include \"Terse.hpp\"");
#endif
    
    template <typename T> friend class Terse;

public:
    /**
     * @brief Initializes an empty Terse object.
     *
     * Data can be appended to an empty Terse object. The first dataset to be pushed in
     * determines size, signedness and dimensions of the remaining datasets that can be pushed in.
     */
    Terse() noexcept {};

    /**
     * @brief Creates a Terse object from data (which can be a std::vector, Field, etc.).
     * Only containers of integral types are allowed. If the container has a member function dim(),
     * that will set the dimensions of the Terse object. Otherwise, the dimensions can be set once
     * using the dim(vector const&) member function.
     * If the input container is passed as an r-value, it will be consumed, and will be empty after the call.
     * If the input container is passed as an r-value, and the include file Concurrent.hpp was included before
     * the Terse.hpp file, compression will be performed concurrently in the background.
     *
     * @tparam C The type of the container containing integral data.
     * @param data The container containing integral data.
     */
    template <Container C>
    Terse(C&& data, Terse_mode const mode = Terse_mode::Default) noexcept {
         insert(0, std::forward<C>(data), mode);
    };

    /**
     * @brief Creates a Terse object given a starting iterator or pointer and the number of elements that need to be encoded.
     *
     * @tparam Iterator The type of the iterator.
     * @param data The starting iterator or pointer to the data.
     * @param size The number of elements to be encoded.
     * @param block The block size for compression (default is 12).
     */
    template <typename Iterator>
    Terse(Iterator const data, size_t const size, unsigned int const block=12, Terse_mode mode = Terse_mode::Default) noexcept :
    d_signed(std::is_signed_v<typename std::iterator_traits<Iterator>::value_type>),
    d_block(block),
    d_size(size) {
        push_back(data, size, mode);
    }
    
    /**
     * @brief Reads in a Terse object that has been written to a stream .
     *
     * Scans the stream for the Terse XML header, then reads the binary Terse data, leaving the stream position exactly one byte beyond the binary Terse data.
     *
     * @tparam STREAM The type of the input stream where the Terse data are stored (e.g. a file stream or a string stream).
     * @param istream The input stream containing Terse data.
     */
    template <typename STREAM> requires std::derived_from<STREAM, std::istream>
    Terse(STREAM& istream) : Terse(istream, XML_element(istream, "Terse")) {};
            
    /**
     * @brief Adds another frame to the Terse object. The new frame is defined by its begin iterator and size.
     *
     * The size must be the same as that of the first frame that was used for creating the Terse object.
     *
     * @tparam Iterator The type of the iterator.
     * @param pos The location where the data need to be inserted.
     * @param data The starting iterator or pointer to the data.
     * @param size The number of elements to be encoded.
     */
    template <typename Iterator>
    void insert(std::size_t const pos, Iterator const data, size_t const size, Terse_mode mode = Terse_mode::Default) noexcept {
        insert(pos, std::span(data, data + static_cast<std::ptrdiff_t>(size)), mode);
    }
    
    /**
     * @brief Inserts a frame into the Terse object. The new frame is defined by a reference to a container.
     * The size and dimensions of the input container must be the same as that of the first frame that was used
     * for creating the Terse object, unless the Terse object is empty, in which case the inserted container determines
     * the size and dimension of subsequent frames that are added.
     * If the input container is passed as an r-value, it will be consumed, and will be empty after the call.
     * If the input container is passed as an r-value, and the include file Concurrent.hpp was included before
     * the Terse.hpp file, compression will be performed concurrently in the background.
     *
     * @tparam C The type of the container containing integral data.
     * @param pos The location where the data need to be inserted.
     * @param data The container containing integral data.
     * @throws std::invalid_argument If the dimensions of this Terse object and the provided frame  differ.
     */
    template <Container C>
    void insert(std::size_t const pos, C&& data, Terse_mode mode = Terse_mode::Default) {
        bool dim_ok = true;
        if constexpr (requires(C& c) { c.dim(); }) {
            for (std::size_t i = 0; i != data.dim().size(); ++i)
                if (number_of_frames() == 0)
                    d_dim.push_back(static_cast<std::size_t>(data.dim()[i]));
                else
                    dim_ok = dim_ok && d_dim[i] == static_cast<std::size_t>(data.dim()[i]);
        }
        else if constexpr(requires (C &c) {c.request().shape;}) {
            auto shape = data.request().shape;
            if (number_of_frames() == 0)
                d_dim = std::vector<std::size_t>(shape.begin(), shape.end());
            else
                dim_ok = dim_ok && d_dim == std::vector<std::size_t>(shape.begin(), shape.end());
        }
        if (!dim_ok)
            throw(std::invalid_argument("The provided container and the requested frame have different dimensions"));
        using T = std::remove_cv_t<std::remove_reference_t<decltype(*data.data())>>;
        if (std::is_signed_v<T>)
            mode = Terse_mode::Signed;
        d_prolix_bits = std::max(d_prolix_bits, 8 * static_cast<unsigned>(sizeof(T)));
        if (number_of_frames() == 0) {
            d_size = data.size();
            d_signed = std::is_signed_v<T>;
        }
        d_metadata.insert(d_metadata.begin() + static_cast<std::ptrdiff_t>(pos), "");
        auto at = d_terse_frames.begin() + static_cast<std::ptrdiff_t>(pos);
        if constexpr (std::is_lvalue_reference_v<C&&>)
            d_terse_frames.insert(at, f_compress(mode, data.data()));
        else if constexpr (std::is_same_v<CONCURRENT, Concurrent>)
            d_terse_frames.insert(at, d_concurrent->background([this, d = std::move(data), mode] { return f_compress(mode, d.data()); }));
        else {
            d_terse_frames.insert(at, f_compress(mode, data.data()));
            [[maybe_unused]] auto local_data = std::move(data);
        }
    }

    /**
     * @brief Insert a (potentially multiframe) Terse object at the specified position.
     * The size and dimensions of both Terse objects must be the same as that of the first frame that was used
     * for creating the Terse object, unless the Terse object is empty, in which case the inserted container determines
     * the size and dimension of subsequent frames that are added.
     *
     * @tparam T Either void or Concurrent.
     * @param pos The location where the data need to be inserted.
     * @param trs The Terse object to be appended.
     * @throws std::out_of_range If the provided Terse object index is greater than or equal to the number of frames.
     * @throws std::invalid_argument If the provided Terse object differs in signedness, size, block size of dimensionality from this Terse object.
     */
    template <typename T>
    void insert(std::ptrdiff_t const pos, Terse<T>& trs) {
        if (number_of_frames() == 0) {
            d_signed = trs.d_signed;
            d_block = trs.d_block;
            d_size = trs.d_size;
            d_dim = trs.d_dim;
        }
        if (static_cast<std::size_t>(pos) > number_of_frames()) throw std::out_of_range("Frame index is out of range.");
        if (trs.d_dim != d_dim)
            throw(std::invalid_argument("Dimension mismatch of the provided Terse object"));
        if (trs.d_signed != d_signed)
            throw(std::invalid_argument("Sign mismatch of the provided Terse object"));
        if (trs.d_block != d_block)
            throw(std::invalid_argument("Blocksize mismatch of the provided Terse object"));
        if (trs.d_size != d_size)
            throw(std::invalid_argument("Size mismatch of the provided Terse object"));
        d_metadata.insert(d_metadata.begin() + pos, trs.d_metadata.begin(), trs.d_metadata.end());
        shrink_to_fit();
        trs.shrink_to_fit();
        if constexpr (std::is_same_v<T, void>)
            d_terse_frames.insert(d_terse_frames.begin() + pos, trs.d_terse_frames.begin(), trs.d_terse_frames.end());
        else {
            std::size_t i_end = trs.d_terse_frames.size();
            for (size_t i = 0; i < i_end; ++i) {
                auto& frame = trs.d_terse_frames[i];
                if (std::holds_alternative<std::vector<uint8_t>>(frame))
                    d_terse_frames.insert(d_terse_frames.begin() + pos + i, std::get<std::vector<uint8_t>>(frame));
                else
                    d_terse_frames.insert(d_terse_frames.begin() + pos + i, std::get<std::future<std::vector<uint8_t>>>(frame).get());
            }
        }
    }

    /**
     * @brief Appends a frame to the Terse object.
     *
     * @tparam Iterator The type of the iterator pointing to the data.
     * @param it The iterator pointing to the data.
     * @param size The number of elements in the frame (must be equal to the size of previously inserted frames).
     */
     template <typename Iterator>
    void push_back(Iterator const it, size_t const size, Terse_mode mode = Terse_mode::Default) noexcept {
        insert(d_terse_frames.size(), it, size, mode);
    }
     
    /**
     * @brief Appends a frame to the Terse object. The new frame is defined by a reference to a container.
     * The size and dimensions of the input container must be the same as that of the first frame that was used
     * for creating the Terse object, unless the Terse object is empty, in which case the inserted container determines
     * the size and dimension of subsequent frames that are added.
     * If the input container is passed as an r-value, it will be consumed, and will be empty after the call.
     * If the input container is passed as an r-value, and the include file Concurrent.hpp was included before
     * the Terse.hpp file, compression will be performed concurrently in the background.
     *
     * @tparam C The type of the container containing integral data.
     * @param data The container containing integral data.
     */
    template <Container C>
    void push_back(C&& data, Terse_mode mode = Terse_mode::Default) noexcept {
        insert(number_of_frames(), std::forward<C>(data), mode);
    }

    /**
     * @brief Appends a (potentially multiframe) Terse object.
     * The size and dimensions of bothe Terse objects must be the same as that of the first frame that was used
     * for creating the Terse object, unless the Terse object is empty, in which case the inserted container determines
     * the size and dimension of subsequent frames that are added.
     *
     * @tparam T Either void or Concurrent.
     * @param trs The Terse object to be appended.
     */
    template <typename T>
    void push_back(Terse<T>& trs) noexcept { insert(static_cast<std::ptrdiff_t>(number_of_frames()), trs); }

    /**
     * @brief Removes one of the frames from the Terse object. Also waits until concurrent compression has finished, and release unused storage to the heap.
     *
     * @param i The index of the frame to be removed.
    */
    void erase(std::ptrdiff_t i) noexcept {
        shrink_to_fit();
        d_metadata.erase(d_metadata.begin() + i);
        d_terse_frames.erase(d_terse_frames.begin() + i);
    }
    
    /**
     * @brief Returns a selected frame as a Terse object.
     *
     * @param pos The index of the selected frame.
     * @throws std::out_of_range If the provided frame index is greater than or equal to the number of frames.
    */
    Terse at(std::size_t pos) {
        if (pos >= number_of_frames()) throw std::out_of_range("Frame index is out of range.");
        Terse result;
        result.d_signed = d_signed;
        result.d_block = d_block;
        result.d_size = d_size;
        result.d_prolix_bits = d_prolix_bits;
        result.d_dim = d_dim;
        result.d_metadata.push_back(d_metadata[pos]);
        result.d_terse_frames.push_back(f_get_frame(pos));
        return result;
    }

    /**
     * @brief Unpacks the Terse data of the requested frame and stores it in the provided container.
     *
     * @tparam C The type of the container.
     * @param container The container where the data will be stored.
     * @param frame The index of the frame to unpack (default is 0).
     * @throws std::invalid_argument If the provided container does not have enough space to unpack all frames,
     * or the number of dimensions of the requested frame and the provided container differ.
     */
    template <Container C>
    C&& prolix(C&& container, std::size_t frame) {
        if (this->size() != container.size())
            throw(std::invalid_argument("The provided container not have enough space for the requested frame of the Terse object"));
        bool dim_ok = true;
        if constexpr(requires (C &c) {c.dim();})
            for (int i = 0; i != d_dim.size(); ++i)
                dim_ok = dim_ok && d_dim[i] == container.dim()[i];
        else if constexpr(requires (C &c) {c.request().shape;})
            dim_ok = dim_ok && d_dim == container.request().shape;
        if (!dim_ok)
            throw(std::invalid_argument("The provided container and the requested frame have different dimensions"));
        prolix(container.data(), frame);
        return std::forward<C>(container);
    }
    
    /**
     * @brief Unpacks all the Terse data and stores these consecutively in the provided container of
     * numerical values.
     *
     * @tparam C The type of the container.
     * @param container The container where the data will be stored.
     * @throws std::invalid_argument If the provided container does not have enough space to unpack all frames.
     */
    template <Container C> requires std::is_arithmetic_v<typename std::remove_cvref_t<C>::value_type>
    C&& prolix(C&& container) {
        if (size() == container.size() && number_of_frames() != 0)
            return prolix(std::forward<C>(container), 0);
        if (size() * number_of_frames() != container.size())
            throw(std::invalid_argument("The provided container not have enough space for unpacking all frames of the Terse object"));
        auto* data_ptr = [&]() {
            if constexpr (requires (C &c) { c.mutable_data(); }) return container.mutable_data();
            else return container.data();
        }();
        if constexpr (std::is_same_v<CONCURRENT, void>)
            for (std::size_t i = 0; i != number_of_frames(); ++i)
                prolix(data_ptr + i * size(), i);
        else {
            std::vector<std::future<void>> futures;
            for (std::size_t i = 0; i != number_of_frames(); ++i)
                futures.push_back(d_concurrent->background([this, i, &data_ptr] {
                    prolix(data_ptr + i * size(), i);
                }));
            for (auto& future : futures) future.get();
        }
        return std::forward<C>(container);
    }

    /**
     * @brief Unpacks all the Terse data and stores it in the provided container of containers.
     * Each frame is stored separately in each sub-container.
     *
     * @tparam C The type of the container.
     * @param container The container where the data will be stored.
     * @throws std::invalid_argument If the provided container does not have enough sub-containers to unpack all frames.
     */
    template <Container C> requires Container<typename std::remove_cvref_t<C>::value_type>
    C&& prolix(C&& container) {
        if (number_of_frames() == 1)
            prolix(container[0], 0);
        else {
            if (container.size() < number_of_frames())
                throw(std::invalid_argument("The provided container does not have enough subcontainers for unpacking all frames of the Terse object"));
            if constexpr (std::is_same_v<CONCURRENT, void>)
                for (std::size_t i = 0; i != number_of_frames(); ++i)
                    prolix(container[i], i);
            else {
                std::vector<std::future<void>> futures;
                for (std::size_t i = 0; i != number_of_frames(); ++i)
                    futures.push_back(d_concurrent->background([this, &container, i] {
                        prolix(container[i], i);
                    }));
                for (auto& future : futures) future.get();
            }
        }
        return std::forward<C>(container);
    }
    
    /**
     * @brief Unpacks the Terse data, storing the unpacked data from the location defined by 'begin'.
     *
     * Unpacks with bounds checking.
     *
     * @tparam Iterator The type of the iterator.
     * @param begin The starting iterator or pointer where the data will be stored.
     * @param frame The index of the frame to unpack (default is 0).
     * @throws std::out_of_range If the provided frame index is greater than or equal to the number of frames.
     * @throws std::invalid_argument If the iterator refers unsigned values, when the Terse object contains signed values.
     */
    template <typename Iterator> requires requires (Iterator& i) {*i;}
    void prolix(Iterator begin, std::size_t frame = 0) {
        if (frame >= number_of_frames()) throw std::out_of_range("Frame index is out of range.");
        if (is_signed() && std::unsigned_integral<typename std::iterator_traits<Iterator>::value_type>)
            throw std::invalid_argument("Cannot decompress signed data into an unsigned container.");
        switch (d_block) {
            case(8)  : return f_prolix<8> (begin, frame);
            case(9)  : return f_prolix<9> (begin, frame);
            case(10) : return f_prolix<10> (begin, frame);
            case(11) : return f_prolix<11> (begin, frame);
            case(12) : return f_prolix<12> (begin, frame);
            case(13) : return f_prolix<13> (begin, frame);
            case(14) : return f_prolix<14> (begin, frame);
            case(15) : return f_prolix<15> (begin, frame);
            case(16) : return f_prolix<16> (begin, frame);
            case(20) : return f_prolix<20> (begin, frame);
            case(24) : return f_prolix<24> (begin, frame);
            case(32) : return f_prolix<32> (begin, frame);
            default  : return f_prolix<0> (begin, frame);
        }
    }

    /**
     * @brief Returns the number of encoded elements of a single frame (all frames in a Terse object must have the
     * same number of elements).
     *
     * @return The number of encoded elements.
     */
    std::size_t size() const noexcept { return d_size; }
    
    /**
     * @brief Returns the number of frames stored in the Terse object.
     *
     * @return The number of frames stored in the Terse object.
     */
    std::size_t number_of_frames() const noexcept { return d_terse_frames.size(); }
    
    /**
     * @brief Returns the dimensions of each of the Terse frames (all frames in a Terse object must have the same dimensions).
     *
     * @return The dimensions of each of the Terse frames.
     */
    std::vector<std::size_t> const& dim() const noexcept { return d_dim; }
    
    /**
     * @brief Sets the dimensions of the Terse frames. All frames must have the same dimensions. If the Terse object is empty,
     * the dimensions can be set to any value. Otherwise, the the total number of elements defined per frame is fixed.
     *
     * @param dim The dimensions to set for the Terse frames.
     * @throws std::invalid_argument If the provided dimensions differ from dimensions that were set earlier.
     */
    void dim(std::vector<std::size_t> const& dim) {
        if (d_dim.size() == 0 || size() == std::accumulate(dim.begin(), dim.end(), std::size_t{1}, std::multiplies<>()))
            d_dim = dim;
        if (dim != d_dim)
            throw std::invalid_argument("Dimensions already set.");
   }
    
    /**
     * @brief Returns the block size that is used for compression.
     *
     * @return The block size that is used for compression.
     */
    std::size_t const& block_size() const noexcept { return d_block; }
    
    /**
     * @brief Sets the the block size used for compression, provided no frames have been compressed yet.
     *
     * @param block The block size that is used for compression. If the Terse object is not empty, the block size
     * cannot be changed, and the original block size is returned.
     */
    void block_size(std::size_t const& block) noexcept {
        d_block = number_of_frames() == 0 ? block : d_block;
    }
    
    /**
     * @brief Returns the degree of partiality of a Terse<Concurrent> object, or zero for Terse<void>..
     *
     * @return The degree of partiality (0 for sequential compression, 1 for using all cores).
     */
    double dop() const noexcept {
        if constexpr (std::is_same_v<CONCURRENT, Concurrent>)
            return d_concurrent->dop();
        else
            return 0;
    }
    
    /**
     * @brief Sets the degree of partiality for a Terse<Concurrent> object.
     *
     * @param new_dop The new degree of partiality (0 for sequential compression, 1 for using all cores).
     */
    void dop(double new_dop) noexcept requires std::is_same_v<CONCURRENT, Concurrent> {
        for (std::size_t i = 0; i!= d_terse_frames.size(); ++i)
            f_get_frame(i);
        d_concurrent = Concurrent(new_dop);
    }
    
    /**
     * @brief Returns the fractional precision for lossy floating point compression.
     *
     * @return The fractional precision for lossy floating point compression; a number between 1 (loss of all information)
     * and 0 (lossless floating point compression). For fractional precsion of 0.01 indicates a precision of 1%.
     */
    double fractional_precision() const noexcept { return static_cast<double>(1) / (static_cast<std::size_t>(1) << (d_binary_precision - 1)); }
    
    /**
     * @brief Sets the fractional precision for lossy floating point compression.
     *
     * @param frac The fractional precision for lossy floating point compression; a number between 1 (loss of all information)
     * and 0 (lossless floating point compression). For fractional precision of 0.01 indicates a precision of 1%.
     */
    void fractional_precision(double frac) {
        auto precision = 1 + std::ceil(std::log2(1 / frac));
        d_binary_precision = static_cast<std::uint8_t>(std::min(precision, 54.0));
    }

    /**
     * @brief Checks if at least one of the frames in the Terse object has floating point values.
     *
     * @return True  any of the frames has floating point values.
     */
    bool is_float() {
        for (std::size_t i = 0; i != d_terse_frames.size(); ++i)
            if (is_float(i)) return true;
        return false;
    }
    
    /**
     * @brief Checks if the frame with the specified index has floatinging point values.
     *
     * @return True  if the frame with the specified index has floating point values
     */
    bool is_float(std::size_t frame_num) { return (Bitqueue_pop(f_get_frame(frame_num)).pop<18, std::size_t>() == 0b111111111111111010); }
    
    /**
     * @brief Returns if the default mode of compression for unsigned data is set to Terse_mode::Small_unsigned.
     *
     * @return true if the default mode of compression for unsigned data is set to Terse_mode::Small_unsigned.
     */
    bool small() const noexcept { return d_small; }
    
    /**
     * @brief Sets/resets the default mode of compression for unsigned to Terse_mode::Small_unsigned.
     *
     * @param val true: sets default mode for unsigned compression to Terse_mode::Small_unsigned; false sets it to Terse_mode::Unsigned
     */
    void small(bool val) { d_small = val; }
    
    /**
     * @brief Returns if the default mode of compression for unsigned data is set to Terse_mode::Unsigned.
     *
     * @return true if the default mode of compression for unsigned data is set to Terse_mode::Unsigned.
     */
    bool fast() const noexcept { return !small(); }
    
    /**
     * @brief Sets/resets the default mode of compression for unsigned to Terse_mode::Unsigned.
     *
     * @param val true: sets default mode for unsigned compression to Terse_mode::Unsigned; false sets it to Terse_mode::Small_unsigned
     */
    void fast(bool val) { small(!val); }

    /**
     * @brief Returns true if the encoded data are signed, false if unsigned. Signed data cannot be decompressed into unsigned data.
     *
     * @return True if the encoded data are signed, false otherwise.
     */
    bool is_signed() const noexcept {return d_signed;}
    
    /**
     * @brief Returns the bit depth of the data before compression.
     *
     * @return The maximum number of bits per element.
     */
    unsigned bits_per_val() const noexcept {return d_prolix_bits;}
    
    /**
     * @brief Returns the number of bytes used for encoding the Terse data.
     *
     * Mainly useful for reporting compression rates.
     *
     * @return The number of bytes used for encoding the Terse data.
     */
    std::size_t terse_size() noexcept {
        std::size_t memsize = 0;
        for (std::size_t i = 0; i!= d_terse_frames.size(); ++i)
            memsize += f_get_frame(i).size();
        return memsize;
    }
    
    /**
     * @brief Returns the number of bytes that a Terse file of the object would contain.
     *
     * Mainly useful for codecs that use Terse.
     *
     * @return The number of bytes used for storing the Terse file.
     */
    std::size_t file_size() noexcept {
        if (number_of_frames() == 0)
            return 0;
        std::ostringstream oss;
        f_write_metadata(oss);
        return oss.str().size() + terse_size();
    }
    
     /**
     * @brief Write the Terse object to the specified output stream.
     *
     * This member function first writes a small XML header with data required to unpack the Terse object,  then writes
     * any provided Terse metadata, then writes the compressed data to the provided output stream.
     *
     * @param ostream The output stream to which the Terse data will be written.
     */
    void write(std::ostream& ostream) {
        if (number_of_frames() == 0) return;
        f_write_metadata(ostream);
        for (std::size_t i = 0; i!= d_terse_frames.size(); ++i)
            ostream.write(reinterpret_cast<const char*>(f_get_frame(i).data()), static_cast<std::ptrdiff_t>(f_get_frame(i).size()));
        ostream.flush();
    }
    
    /**
     * @brief Releases unused buffer storage to heap memory. This increases available heap memory when Terse objects were constructed
     * from uncompressed data sources held in memory. If compression is performed concurrently, also waits for all compression
     * processes to finish. It has no effect when Terse object are read from a stream.
     */
    void shrink_to_fit() noexcept {
        for (std::size_t i = 0; i!= d_terse_frames.size(); ++i)
            f_get_frame(i).shrink_to_fit();
    }
    
    /**
     * @brief Sets / overwrites any optional metadata that are associated with the specified frame. Metadata are not compressed.
     * Metadata will be written to, and read from a stream as part of the Terse object.
     *
     * @param data The metadata as a string object.
     * @param pos The number of the frame pertaining to the metadata.
     * @throws std::out_of_range If the provided frame index is greater than or equal to the number of frames.
     */
    void metadata(std::size_t const pos, std::string const data) {
        if (pos >= number_of_frames()) throw std::out_of_range("Frame index is out of range.");
        d_metadata[pos] = data;
    }

    void metadata(std::string data) {
        if (number_of_frames() == 0) throw std::invalid_argument("Cannot add metadata, as this Terse object has no frames.");
        d_metadata[0] = data;
    }

    /**
     * @brief Returns the metadata that are associated with the specified frame.
     *
     * @param frame The number of the frame pertaining to the metadata.
     * @return A constant reference to the metadata as a std::string. If no metadata are available, an empty string is returned.
     */
    std::string const& metadata(std::size_t frame = 0) const noexcept {
        return d_metadata[frame];
    }
    
private:
    using FrameStorage = std::conditional_t<
        std::is_same_v<CONCURRENT, Concurrent>,
        std::vector<std::variant<std::future<std::vector<std::uint8_t>>, std::vector<std::uint8_t>>>,
        std::vector<std::vector<std::uint8_t>>>;

    FrameStorage d_terse_frames;
    bool d_signed;
    bool d_small = true;
    std::size_t d_block = 12;
    std::size_t d_size = 0;
    unsigned d_prolix_bits = 0;
    std::uint8_t d_binary_precision = 54;
    std::vector<std::size_t> d_dim;
    std::vector<std::string> d_metadata;
    std::optional<Concurrent> d_concurrent = []() -> std::optional<Concurrent> {
        if constexpr (std::is_same_v<CONCURRENT, Concurrent>) return Concurrent(1);
        else return std::nullopt;
    }();
    
    template <typename STREAM> requires std::derived_from<STREAM, std::istream>
    Terse(STREAM& istream, XML_element const& xmle) {
        if (!istream.eof()) {
            d_prolix_bits = unsigned(std::stoul(xmle.attribute("prolix_bits")));
            d_signed = std::stoul(xmle.attribute("signed"));
            d_block = std::stoul(xmle.attribute("block"));
            d_size = std::stoull(xmle.attribute("number_of_values"));
            std::istringstream dim_str(xmle.attribute("dimensions"));
            unsigned int d;
            while (dim_str >> d)
                d_dim.push_back(d);
            if (xmle.attribute("metadata_string_sizes") != "") {
                std::istringstream meta_str(xmle.attribute("metadata_string_sizes"));
                unsigned int val;
                while (meta_str >> val) {
                    d_metadata.push_back(std::string(val, ' '));
                    istream.read(d_metadata.back().data(), val);
                }
            }
            d_terse_frames.resize(std::stoull(xmle.attribute("number_of_frames")));
            for (auto& frame : d_terse_frames)
                frame = std::vector<std::uint8_t>(); // Initialize each element to an empty vector
            if (xmle.attribute("memory_sizes_of_frames") == "")
                f_fill_terse_frames(istream, std::stoul(xmle.attribute("memory_size")));
            else {
                std::istringstream frame_sizes_str(xmle.attribute("memory_sizes_of_frames"));
                unsigned int val;
                for (std::size_t i = 0; i != d_terse_frames.size(); ++i) {
                    frame_sizes_str >> val;
                    f_get_frame(i).resize(val);
                    istream.read(reinterpret_cast<char*>(&(f_get_frame(i)[0])), static_cast<std::ptrdiff_t>(f_get_frame(i).size()));
                }
            }
        }
    }
    
    void f_write_metadata(std::ostream& ostream) {
        std::size_t memory_size = terse_size();
        XML_element xml("<Terse/>");
        xml.add_attribute("prolix_bits", d_prolix_bits);
        xml.add_attribute("signed", d_signed);
        xml.add_attribute("block", d_block);
        xml.add_attribute("number_of_values", size());
        if (!d_dim.empty()) xml.add_attribute("dimensions", d_dim);
        xml.add_attribute("number_of_frames", d_terse_frames.size());
        std::vector<size_t> frame_sizes;
        for (size_t i = 0; i != d_terse_frames.size(); ++i)
            frame_sizes.push_back(f_get_frame(i).size());
        xml.add_attribute("memory_sizes_of_frames", frame_sizes);
        xml.add_attribute("memory_size", memory_size);
        if (!d_metadata.empty()) {
            std::vector<size_t> metadata_sizes;
            for (size_t i = 0; i != d_metadata.size(); ++i)
                metadata_sizes.push_back(d_metadata[i].size());
            xml.add_attribute("metadata_string_sizes", metadata_sizes);
        }
        ostream << xml.XML();
        for (auto& str : d_metadata) ostream << str;
        ostream.flush();
    }

    std::vector<std::uint8_t>& f_get_frame(std::size_t index) noexcept {
        if constexpr (std::is_same_v<CONCURRENT, void>)
            return d_terse_frames[index];
        else {
            if (std::holds_alternative<std::future<std::vector<std::uint8_t>>>(d_terse_frames[index]))
                d_terse_frames[index] = std::get<std::future<std::vector<std::uint8_t>>>(d_terse_frames[index]).get();
            return std::get<std::vector<std::uint8_t>>(d_terse_frames[index]);
        }
    }

    template <typename Iterator>
    auto f_compress(Terse_mode mode, Iterator const data_begin) {
        if constexpr (std::is_signed_v<std::remove_reference_t<decltype(*data_begin)>>)
            return f_compress<Terse_mode::Signed>(data_begin);
        else switch (mode) {
            case Terse_mode::Unsigned:     return f_compress<Terse_mode::Unsigned>(data_begin);
            case Terse_mode::Small_unsigned:    return f_compress<Terse_mode::Small_unsigned>(data_begin);
            case Terse_mode::Signed:   return f_compress<Terse_mode::Signed>(data_begin);
            default: return d_small ? f_compress<Terse_mode::Small_unsigned>(data_begin) : f_compress<Terse_mode::Unsigned>(data_begin);
        }
    }
    
#define PUSH_BITS(prevbits, bits) \
    if (prevbits == bits) bitqueue.push_back<1>(0b1); \
    else if (bits < 7) bitqueue.push_back<4>(bits << 1); \
    else if (bits < 10) bitqueue.push_back<6>(0b1110 + ((bits - 7) << 4)); \
    else bitqueue.push_back<12>(0b111110 + ((bits - 10) << 6)); \
    prevbits = bits;

    template <Terse_mode MODE, typename Iterator> requires ((MODE == Terse_mode::Signed || MODE == Terse_mode::Unsigned) &&
                                                            std::integral<typename std::iterator_traits<Iterator>::value_type>)
    std::vector<std::uint8_t> const f_compress(Iterator data) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        std::size_t terse_frame_size = static_cast<std::size_t>(d_size * sizeof(decltype(*data)) * 0.01) + 2 * (d_block * sizeof(decltype(*data)) + 32);
        terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
        std::vector<std::uint8_t> terse_frame(terse_frame_size);
        Unique_array<std::remove_const_t<T>> buffer(d_block);
        Bitqueue_push_back bitqueue(terse_frame);
        if constexpr (MODE != Terse_mode::Signed)
            bitqueue.push_back<18>(0b111111111111111000);
        std::size_t prevbits = 0;
        std::size_t prevmasked_bits = 0;
        for (std::size_t from = 0; from < d_size; from += d_block) {
            std::size_t index = static_cast<std::size_t>(bitqueue.data() - terse_frame.data());
            if (index > terse_frame.size() - d_block * sizeof(decltype(*data)) - 32) {
                terse_frame_size = std::max(static_cast<std::size_t>(1.1 * terse_frame.size() * d_size / from),
                                            index + 2 * (d_block * sizeof(decltype(*data)) + 32));
                terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
                terse_frame.resize(terse_frame_size);
                bitqueue.relocate(terse_frame.data() + index);
            }
            std::span<T const> data_block(data + static_cast<std::ptrdiff_t>(from), std::min(d_size - from, d_block));
            std::uint8_t significant_bits = f_most_significant_bit(data_block);
            PUSH_BITS(prevbits, significant_bits);
            if constexpr (MODE == Terse_mode::Signed)
                bitqueue.push_back(significant_bits, data_block);
            else {
                if (significant_bits != sizeof(T) * 8)
                    bitqueue.push_back(significant_bits, data_block);
                else { // masked data
                    std::transform(data_block.begin(), data_block.end(), buffer.begin(), [](T val) {return static_cast<T>(val + 1);});
                    std::uint8_t masked_bits = f_most_significant_bit(std::span(buffer.begin(), buffer.end()));
                    PUSH_BITS(prevmasked_bits, masked_bits);
                    bitqueue.push_back(masked_bits, buffer);
                }
            }
        }
        terse_frame_size = static_cast<std::size_t>(bitqueue - terse_frame.data());
        terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
        terse_frame.resize(terse_frame_size);
        return terse_frame;
    }

    template <Terse_mode MODE, typename Iterator> requires (std::floating_point<typename std::iterator_traits<Iterator>::value_type>)
    std::vector<std::uint8_t> const f_compress(Iterator data) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        std::uint8_t binary_precision = std::min(d_binary_precision, static_cast<std::uint8_t>(std::numeric_limits<T>::digits +1));
        std::size_t terse_frame_size = static_cast<std::size_t>(d_size * sizeof(T) * 0.01) + 2 * (d_block * sizeof(decltype(*data)) + 32);
        terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
        std::vector<std::uint8_t> terse_frame(terse_frame_size);
        Unique_array<std::remove_const_t<T>> buffer(d_block);
        Bitqueue_push_back bitqueue(terse_frame);
        bitqueue.push_back<18>(0b111111111111111010);
        bitqueue.push_back<6>(binary_precision);
        std::size_t prevbits_exponents = 0;
        Unique_array<std::int64_t> mantissas(d_block);
        Unique_array<int> exponents(d_block);
        std::size_t mantissa_bits = (static_cast<std::size_t>(1) << (binary_precision - 1));
        for (std::size_t from = 0; from < d_size; from += d_block) {
            std::size_t index = static_cast<std::size_t>(bitqueue.data() - terse_frame.data());
            if (index > terse_frame.size() - d_block * sizeof(decltype(*data)) - 32) {
                terse_frame_size = std::max(static_cast<std::size_t>(1.1 * terse_frame.size() * d_size / from),
                                            index + 2 * (d_block * sizeof(decltype(*data)) + 32));
                terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
                terse_frame.resize(terse_frame_size);
                bitqueue.relocate(terse_frame.data() + index);
            }
            std::span<T const> data_block(data + static_cast<std::ptrdiff_t>(from), std::min(d_size - from, d_block));
            bool unsigned_block = true;
            for (std::size_t i=0; i != data_block.size(); ++i) {
                T mantissa_float = std::frexp(data_block[i], &exponents[i]);
                unsigned_block = unsigned_block && (mantissa_float >= 0);
                if (!std::isnan(mantissa_float))
                    mantissas[i] = static_cast<std::int64_t>(std::round(mantissa_float * mantissa_bits));
                else {
                    mantissas[i] = 0;
                    exponents[i] = 1 << 12;
                }
            }
            bitqueue.push_back<1>(unsigned_block);
            std::uint8_t significant_bits_exponents = f_most_significant_bit(std::span(exponents.begin(), data_block.size()));
            PUSH_BITS(prevbits_exponents, significant_bits_exponents);
            bitqueue.push_back(binary_precision - unsigned_block, std::span(mantissas.begin(), data_block.size()));
            bitqueue.push_back(significant_bits_exponents, std::span(exponents.begin(), data_block.size()));
        }
        terse_frame_size = static_cast<std::size_t>(bitqueue - terse_frame.data());
        terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
        terse_frame.resize(terse_frame_size);
        return terse_frame;
    }

#undef PUSH_BITS

    template <typename T, std::size_t N> requires std::is_unsigned_v<T>
    constexpr void f_compress_weak_block(std::span<T const, N> const data_block,
                                         Bitqueue_push_back& bitqueue,
                                         T const maxval,
                                         T& previous_maxval)  {
        if (previous_maxval == 0 && maxval == 0)
            bitqueue.push_back<1>(0b1);
        else if (previous_maxval == maxval)
            bitqueue.push_back<2>(0b11);
        else if (previous_maxval < maxval && previous_maxval + 1 == maxval)
            bitqueue.push_back<2>(0b10);
        else if (maxval != 6 && previous_maxval > maxval && previous_maxval - 1 == maxval)
            bitqueue.push_back<2>(0b01);
        else if (previous_maxval == 6 && maxval == 4)
            bitqueue.push_back<2>(0b10);
        else
            bitqueue.push_back<5>(maxval << 2);
        switch (maxval) {
            case 0: break;
            case 1: bitqueue.push_back<1>(data_block); break;
            case 3: bitqueue.push_back<2>(data_block); break;
            default:
                std::size_t mult = 1;
                std::size_t compact = 0;
                for (std::size_t i = 0; i != data_block.size(); ++i) {
                    compact += mult * data_block[i];
                    mult *= (maxval + 1);
                }
                bitqueue.push_back(f_most_significant_bit(mult - 1), compact);
        }
        previous_maxval = maxval;
    }

    template <typename T, std::size_t N>
    constexpr void f_compress_strong_block(std::span<T, N> const data_block,
                                           Bitqueue_push_back& bitqueue,
                                           std::uint8_t const significant_bits,
                                           std::size_t& previous_significant_bits)  {
        if (previous_significant_bits == significant_bits)
            bitqueue.push_back<2>(0b11);
        else if (previous_significant_bits + 1 == significant_bits)
            bitqueue.push_back<2>(0b10);
        else if (previous_significant_bits - 1 == significant_bits)
            bitqueue.push_back<2>(0b01);
        else if (significant_bits < 10)
            bitqueue.push_back<8>(0b11100 + ((significant_bits - 3) << 5));
        else if (significant_bits < 17)
            bitqueue.push_back<11>(0b11111100 + ((significant_bits - 10) << 8));
        else
            bitqueue.push_back<17>(0b11111111100 + ((significant_bits - 17) << 11));
        bitqueue.push_back(significant_bits, data_block);
        previous_significant_bits = significant_bits;
    }
    
    template <Terse_mode MODE, typename Iterator> requires (MODE == Terse_mode::Small_unsigned)
    std::vector<std::uint8_t> const f_compress(Iterator data) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        static_assert(std::is_unsigned_v<T>, "Cannot compress signed data with Terse_mode::small");
        //std::size_t const block = std::min(d_block, 24ul);
        std::size_t block = std::min(d_block, std::size_t(24));
        std::size_t terse_frame_size = static_cast<std::size_t>(d_size * sizeof(decltype(*data)) * 0.01) + 2 * (d_block * sizeof(decltype(*data)) + 32);
        terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
        std::vector<std::uint8_t> terse_frame(terse_frame_size);
        Bitqueue_push_back bitqueue(terse_frame);
        bitqueue.push_back<18>(0b111111111111111100);
        T prevmax = 0;
        std::size_t prevbits = 0;
        for (std::size_t from = 0; from < d_size; from += block) {
            std::size_t index = static_cast<std::size_t>(bitqueue.data() - terse_frame.data());
            if (index > terse_frame.size() - block * sizeof(decltype(*data)) - 32) {
                terse_frame_size = std::max(static_cast<std::size_t>(1.1 * terse_frame.size() * d_size / from),
                                            index + 2 * (d_block * sizeof(decltype(*data)) + 32));
                terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
                terse_frame.resize(terse_frame_size);
                bitqueue.relocate(terse_frame.data() + index);
            }
            std::span<T const> data_block(data + static_cast<std::ptrdiff_t>(from), std::min(d_size - from, d_block));
            T max = std::ranges::max(data_block);
            if (max < 7) {
                f_compress_weak_block(data_block, bitqueue, max, prevmax);
                prevbits = 65;
            }
            else {
                std::uint8_t const significant_bits = f_most_significant_bit(max);
                if (significant_bits == sizeof(T) * 8)
                    f_compress_masked(data, terse_frame, from, bitqueue, max, prevmax, prevbits);
                else {
                    f_compress_strong_block(data_block, bitqueue, significant_bits, prevbits);
                    prevmax = std::numeric_limits<T>::max() / 2;
                }
            }
        }
        terse_frame_size = static_cast<std::size_t>(bitqueue - terse_frame.data()) + 8;
        terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
        terse_frame.resize(terse_frame_size);
        return terse_frame;
    }
    
    template <typename Iterator, typename T>
    void f_compress_masked(Iterator data,
                           std::vector<std::uint8_t>& terse_frame,
                           std::size_t& from,
                           Bitqueue_push_back& bitqueue, T& max, T& prevmax, std::size_t& prevbits) noexcept {
        //std::size_t const block = std::min(d_block, 24ul);
        std::size_t block = std::min(d_block, std::size_t(24));
        Unique_array<T> buffer(block);
        if constexpr (sizeof(T) * 8 < 10)
            bitqueue.push_back<8>(0b11100 + ((sizeof(T) * 8 - 3) << 5));
        else if constexpr (sizeof(T) * 8 < 17)
            bitqueue.push_back<11>(0b11111100 + ((sizeof(T) * 8 - 10) << 8));
        else
            bitqueue.push_back<17>(0b11111111100 + ((sizeof(T) * 8 - 17) << 11));
        prevmax = std::numeric_limits<T>::max();
        prevbits = sizeof(T) * 8 + 2;
        for ( ; from < d_size; from += block) {
            std::size_t index = static_cast<std::size_t>(bitqueue.data() - terse_frame.data());
            if (index > terse_frame.size() - block * sizeof(decltype(*data)) - 32) {
                std::size_t terse_frame_size = std::max(static_cast<std::size_t>(1.1 * terse_frame.size() * d_size / from),
                                                        index + 2 * (block * sizeof(decltype(*data)) + 32));
                terse_frame_size = (terse_frame_size + 7) & ~std::size_t(7);
                terse_frame.resize(terse_frame_size);
                bitqueue.relocate(terse_frame.data() + index);
            }
            std::size_t to = std::min(d_size, from + block);
            std::transform(data + static_cast<std::ptrdiff_t>(from), data + static_cast<std::ptrdiff_t>(to), buffer.begin(), [](T val) {return static_cast<T>(val + 1);});
            std::span<T const> unmasked(buffer.begin(), buffer.begin() + static_cast<std::ptrdiff_t>(to - from));
            max = std::ranges::max(unmasked);
            if (max < 7) {
                f_compress_weak_block(unmasked, bitqueue, max, prevmax);
                prevbits = 65;
            }
            else {
                f_compress_strong_block(unmasked, bitqueue, f_most_significant_bit(max), prevbits);
                prevmax = max;
            }
            if (to != d_size) {
                std::span test_for_masked(data + static_cast<std::ptrdiff_t>(to), std::min(d_size - to, block));
                if (std::ranges::max(test_for_masked) != std::numeric_limits<T>::max()) {
                    bitqueue.push_back<1>(0); // End masking
                    break;
                 }
                bitqueue.push_back<1>(1);
            }
        }
    }

#define POP(T, bits, begin, num)                                                    \
 do { if constexpr (N == 0)                                                      \
        bitqueue.pop(bits, std::span(begin, begin + num));                       \
    else                                                                         \
        switch (bits + (num != static_cast<std::ptrdiff_t>(d_block) ? 9 : 0)) {  \
            case 0: bitqueue.template pop<0>(std::span<T, N>(begin, N)); break;  \
            case 1: bitqueue.template pop<1>(std::span<T, N>(begin, N)); break;  \
            case 2: bitqueue.template pop<2>(std::span<T, N>(begin, N)); break;  \
            case 3: bitqueue.template pop<3>(std::span<T, N>(begin, N)); break;  \
            case 4: bitqueue.template pop<4>(std::span<T, N>(begin, N)); break;  \
            case 5: bitqueue.template pop<5>(std::span<T, N>(begin, N)); break;  \
            case 6: bitqueue.template pop<6>(std::span<T, N>(begin, N)); break;  \
            case 7: bitqueue.template pop<7>(std::span<T, N>(begin, N)); break;  \
            case 8: bitqueue.template pop<8>(std::span<T, N>(begin, N)); break;  \
            default: bitqueue.pop(bits, std::span(begin, begin + num));          \
} } while (0)

#define BITS(bits) \
if (bitqueue.template pop<1,uint8_t>() == 0) {          \
    bits = bitqueue.template pop<3,uint8_t>();          \
    if (bits == 7) {                                    \
        bits += bitqueue.template pop<2,uint8_t>();     \
        if (bits == 10)                                 \
            bits += bitqueue.template pop<6,uint8_t>(); \
    }                                                   \
}

    template <std::size_t N, typename Iterator>
    void f_prolix(Iterator const begin, std::size_t const frame) noexcept {
        Bitqueue_pop bitqueue(f_get_frame(frame));
        std::size_t flag = bitqueue.pop<18, std::size_t>();
        switch (flag) {
            case 0b111111111111111100: return f_prolix_small_unsigned<N>(begin, frame);
            case 0b111111111111111000: return f_prolix_unsigned<N>(begin, frame);
            case 0b111111111111111010: return f_prolix_float<N>(begin, frame);
            default: return f_prolix_signed<N>(begin, frame);
        }
    }
    
    template <std::size_t N, typename Iterator>
    void f_prolix_signed(Iterator begin, std::size_t frame) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        Bitqueue_pop bitqueue(f_get_frame(frame));
        uint8_t significant_bits = 0;
        for (std::size_t from = 0; from < d_size; from += d_block) {
            auto const to = std::min(d_size, from + d_block);
            BITS(significant_bits);
            if constexpr (std::is_integral_v<T>)
                POP(T, significant_bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
            else
                f_pop_into<N>(bitqueue, significant_bits, begin + from, static_cast<std::ptrdiff_t>(to - from));
        }
    }

    template <std::size_t N, typename Iterator> 
    void f_prolix_float(Iterator begin, std::size_t frame) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        Bitqueue_pop bitqueue(f_get_frame(frame));
        bitqueue.pop<18, std::size_t>();
        std::uint8_t const binary_precision = bitqueue.pop<6, std::uint8_t>();
        std::uint8_t significant_bits_exponents = 0;
        Unique_array<std::int64_t> mantissas(d_block);
        Unique_array<std::int16_t> exponents(d_block);
        std::size_t mantissa_bits = (static_cast<std::size_t>(1) << (binary_precision - 1));
        for (size_t from = 0; from < d_size; from += d_block) {
            auto const to = std::min(d_size, from + d_block);
            std::uint8_t unsigned_block = bitqueue.pop<1, std::uint8_t>();
            BITS(significant_bits_exponents);
            if (unsigned_block)
                POP(std::uint64_t, binary_precision - 1, reinterpret_cast<std::uint64_t*>(mantissas.data()), static_cast<std::ptrdiff_t>(to - from));
            else
                POP(std::int64_t, binary_precision, mantissas.begin(), static_cast<std::ptrdiff_t>(to - from));
            POP(std::int16_t, significant_bits_exponents, exponents.begin(), static_cast<std::ptrdiff_t>(to - from));
            for (std::size_t i = 0; i != static_cast<std::size_t>(to - from); ++i) {
                if (exponents[i] == 1 << 12) {
                    if constexpr (std::integral<T>) *begin++ = std::numeric_limits<T>::max();
                    else *begin++ = std::numeric_limits<T>::quiet_NaN();
                }
                else {
                    double const val = std::ldexp(static_cast<double>(mantissas[i]) / mantissa_bits, exponents[i]);
                    if constexpr (std::floating_point<T>)
                        *begin++ = static_cast<T>(val);
                    else
                        *begin++ = static_cast<T>(std::round(std::max(std::min(static_cast<double>(std::numeric_limits<T>::max()), val), val)));
                }
            }
        }
    }
    
    template <std::size_t N, typename Iterator>
    void f_prolix_unsigned(Iterator begin, std::size_t frame) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        Bitqueue_pop bitqueue(f_get_frame(frame));
        bitqueue.pop<18, std::size_t>();
        uint8_t significant_bits = 0;
        uint8_t masked_bits = 0;
        for (std::size_t from = 0; from < d_size; from += d_block) {
            auto const to = std::min(d_size, from + d_block);
            BITS(significant_bits);
            if (significant_bits != d_prolix_bits) {
                if constexpr (std::is_integral_v<T>)
                    POP(T, significant_bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
                else
                    f_pop_into<N>(bitqueue, significant_bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
            }
            else {
                BITS(masked_bits);
                if constexpr (std::is_integral_v<T>)
                    POP(T, masked_bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
                else
                    f_pop_into<N>(bitqueue, masked_bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
                for (std::ptrdiff_t i = static_cast<std::ptrdiff_t>(from); i != static_cast<std::ptrdiff_t>(to); ++i)
                    --begin[i];
            }
        }
    }

    template <std::size_t N, typename Iterator>
    requires std::floating_point<typename std::iterator_traits<Iterator>::value_type>
    void f_prolix_small_unsigned(Iterator begin, std::size_t frame) noexcept {
        using F = typename std::iterator_traits<Iterator>::value_type;
        auto prolix_float = [&](auto type_tag) {
            using T = decltype(type_tag);
            std::vector<T> buffer(d_size);
            prolix(buffer.begin(), frame);
            std::transform(buffer.begin(), buffer.end(), begin, [](T val) { return static_cast<F>(val); });
        };
        if (d_prolix_bits <= 8)        prolix_float(uint8_t{});
        else if (d_prolix_bits <= 16)  prolix_float(uint16_t{});
        else if (d_prolix_bits <= 32)  prolix_float(uint32_t{});
        else                           prolix_float(uint64_t{});
    }
    
    constexpr std::size_t f_integer_power(std::size_t base, std::size_t exp) {
        std::size_t result = 1;
        while (exp) {
            if (exp & 1) result *= base;  // If the exponent is odd, multiply by base
            base *= base;  // Square the base
            exp >>= 1;  // Divide exponent by 2
        }
        return result;
    }
    
    template <std::size_t N, typename Iterator> requires std::integral<typename std::iterator_traits<Iterator>::value_type>
    void f_prolix_small_unsigned(Iterator begin, std::size_t frame) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        //std::size_t block = std::min(d_block, 24ul);
        std::size_t block = std::min(d_block, std::size_t(24));
        Bitqueue_pop bitqueue(f_get_frame(frame));
        bitqueue.pop<18, std::size_t>();
        uint8_t bits = 0;
        T max = 0;
        for (size_t from = 0; from < d_size; from += block) {
            auto const to = std::min(d_size, from + block);
            f_get_max(bitqueue, max, bits);
            switch (max) {
                case 0: POP(T, 0, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from)); break;
                case 1: POP(T, 1, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from)); break;
                case 2: {
                    std::span<T> data_block(begin + static_cast<std::ptrdiff_t>(from), begin + static_cast<std::ptrdiff_t>(to));
                    std::size_t val = bitqueue.pop<std::size_t>(f_most_significant_bit(f_integer_power(3, data_block.size()) - 1));
                    for (std::size_t i = 0; i != data_block.size(); ++i) {
                        data_block[i] = val % (3);
                        val /= 3;
                    }
                    break;
                }
                case 3: POP(T, 2, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from)); break;
                case 7: POP(T, 3, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from)); break;
                default: {
                    if (max < 7) {
                        std::span<T> data_block(begin + static_cast<std::ptrdiff_t>(from), begin + static_cast<std::ptrdiff_t>(to));
                        std::size_t const mult = f_integer_power(static_cast<std::size_t>(max) + 1, data_block.size()) - 1;
                        std::size_t val = bitqueue.pop<std::size_t>(f_most_significant_bit(mult));
                        for (std::size_t i = 0; i != data_block.size(); ++i) {
                            auto [quot, rem] = std::div(static_cast<std::ptrdiff_t>(val), static_cast<std::ptrdiff_t>(max + 1));
                            data_block[i] = static_cast<T>(rem);
                            val = static_cast<std::size_t>(quot);
                        }
                    }
                    else if (bits == d_prolix_bits)
                        f_prolix_small_unsigned_masked<N>(bitqueue, begin, from, max, bits);
                    else
                        POP(T, bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
                }
            }
        }
    }

    template <std::size_t N, typename Iterator>
    void f_prolix_small_unsigned_masked(Bitqueue_pop& bitqueue, Iterator begin, std::size_t& from, auto& max, auto& bits) noexcept {
        using T = typename std::iterator_traits<Iterator>::value_type;
        //std::size_t block = std::min(d_block, 24ul);
        std::size_t block = std::min(d_block, std::size_t(24));
        Unique_array<T> buffer(block);
        bits = 0;
        for (uint8_t masked = true; from < d_size; from += block) {
            auto const to = std::min(d_size, from + block);
            f_get_max(bitqueue, max, bits);
            if (max >= 7) {
                POP(T, bits, begin + static_cast<std::ptrdiff_t>(from), static_cast<std::ptrdiff_t>(to - from));
                max = std::ranges::max(std::span(begin + static_cast<std::ptrdiff_t>(from), to - from));
                for (std::ptrdiff_t i = static_cast<ptrdiff_t>(from); i != static_cast<std::ptrdiff_t>(to); ++i)
                    --begin[i];
            }
            else {
                std::span<T> data_block(begin + static_cast<std::ptrdiff_t>(from), begin + static_cast<std::ptrdiff_t>(to));
                std::size_t const mult = f_integer_power(static_cast<std::size_t>(max) + 1, data_block.size()) - 1;
                std::size_t val = bitqueue.pop<std::size_t>(f_most_significant_bit(mult));
                for (std::size_t i = 0; i != data_block.size(); ++i) {
                    auto [quot, rem] = std::div(static_cast<std::ptrdiff_t>(val), static_cast<std::ptrdiff_t>(max + 1));
                    data_block[i] = static_cast<T>(rem) - 1;
                    val = static_cast<std::size_t>(quot);
                }
            }
            masked = bitqueue.pop<1, uint8_t>();
            if (!masked)
                break;
        }
    }
    
    template <std::size_t N, typename Iterator>
    void f_pop_into(Bitqueue_pop& bitqueue, uint8_t significant_bits, Iterator begin, std::ptrdiff_t const num) noexcept {
        std::vector<std::int64_t> tmp(static_cast<std::size_t>(num));
        if (is_signed())
            POP(std::int64_t, significant_bits, tmp.data(), num);
        else
            POP(std::uint64_t, significant_bits, reinterpret_cast<std::uint64_t*>(tmp.data()), num);
        std::copy(tmp.begin(), tmp.end(), begin);
    }
    
#undef POP
#undef BITS

    template <typename T>
    inline void f_get_max(Bitqueue_pop& bitqueue, T& max, std::uint8_t& bits) const noexcept {
        uint8_t flag = bitqueue.template pop<1,uint8_t>();
        if (flag == 1 && max == 0) return;
        flag = static_cast<uint8_t>(flag << 1) + bitqueue.template pop<1,uint8_t>();
        if (flag == 0b11) return;
        if (flag == 0b10) {
            --bits;
            --max;
        }
        else if (flag == 0b01) {
            ++bits;
            max = (max == 6) ? max - 2 : max + 1;
        }
        else {
            max = static_cast<T>(bits = bitqueue.template pop<3,std::uint8_t>());
            if (bits == 7) {
                max = std::numeric_limits<T>::max() / 2;
                bits = 3 + bitqueue.template pop<3,std::uint8_t>();
                if (bits == 10) {
                    bits += bitqueue.template pop<3,std::uint8_t>();
                    if (bits == 17)
                        bits += bitqueue.template pop<6,std::uint8_t>();
                }
            }
        }
    }
    
    template <typename T0, std::size_t N> requires std::integral<T0>
    constexpr inline std::uint8_t f_most_significant_bit(std::span<T0,N> const values) const noexcept {
        std::size_t setbits = 0;
        if constexpr (std::is_unsigned_v<T0>)
            for (auto const& val : values)
                setbits |= val;
        else
            for (auto const& val : values)
                setbits |= (val == -1) ? std::size_t(1) : static_cast<std::size_t>(std::abs(val)) << 1;
        std::size_t r=0;
        for ( ; setbits; setbits>>=1, ++r);
        return static_cast<std::uint8_t>(std::min(r, sizeof(T0) * 8));
    }
    
    template <typename T0>
    constexpr inline std::uint8_t f_most_significant_bit(T0 val) const noexcept {
        if constexpr (std::is_signed_v<T0>)
            val = (val == -1) ? 1 : std::abs(val) << 1;
        std::size_t r=0;
        for ( ; val; val>>=1, ++r);
        return static_cast<std::uint8_t>(std::min(r, sizeof(T0) * 8));
    }
    
    template <typename STREAM> requires std::derived_from<STREAM, std::istream>
    void f_fill_terse_frames(STREAM& istream, std::size_t const number_of_bytes) {
        f_get_frame(0).resize(number_of_bytes);
        istream.read(reinterpret_cast<char*>(&(f_get_frame(0)[0])), static_cast<std::ptrdiff_t>(number_of_bytes));
        std::vector<std::size_t> terse_sizes(d_terse_frames.size());
        std::uint8_t const* terse_begin = f_get_frame(0).data();
        for (std::size_t current_frame = 0; current_frame != d_terse_frames.size(); ++current_frame) {
            Bitqueue_pop bitqueue(f_get_frame(0));
            uint8_t significant_bits = 0;
            for (std::size_t from = 0; from < size(); from += d_block) {
                if (bitqueue.template pop<1,uint8_t>() == 0) {
                    significant_bits = bitqueue.template pop<3,uint8_t>();
                    if (significant_bits == 7) {
                        significant_bits += bitqueue.template pop<2,uint8_t>();
                        if (significant_bits == 10)
                            significant_bits += bitqueue.template pop<6,uint8_t>();
                    }
                }
                bitqueue.skip(significant_bits * std::min(static_cast<std::size_t>(d_block), size() - from));
            }
            terse_sizes[current_frame] = static_cast<std::size_t>(bitqueue - terse_begin);
            terse_begin += terse_sizes[current_frame];
        }
        std::size_t data_start = terse_sizes[0];
        for (std::size_t frame_num = 1; frame_num != d_terse_frames.size(); ++frame_num) {
            terse_sizes[frame_num] = ((terse_sizes[frame_num] + sizeof(size_t) - 1) / sizeof(size_t)) * sizeof(size_t);
            f_get_frame(frame_num).resize(terse_sizes[frame_num]);
            std::memcpy(f_get_frame(frame_num).data(), f_get_frame(0).data() + data_start, terse_sizes[frame_num]);
            data_start += terse_sizes[frame_num];
        }
        f_get_frame(0).resize(terse_sizes[0]);
        f_get_frame(0).shrink_to_fit();
    }
};
} // end namespace jpa

#endif /* Terse_h */
