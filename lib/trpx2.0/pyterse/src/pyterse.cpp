
 /**
 * @file pyterse.cpp
 * @author Senik Matinyan
 * @date 2024
 * @brief Python bindings for Terse compression library using pybind11
 *
 * This file provides Python bindings for the Terse compression library,
 * allowing seamless integration with NumPy arrays and Python I/O streams.
 */
 
 
 #include <pybind11/pybind11.h>
 #include <pybind11/stl.h>
 #include "Concurrent.hpp"
 #include "Terse.hpp"
 #include <pybind11/functional.h>
 #include <pybind11/numpy.h>
 #include <pybind11/complex.h>
 #include <pybind11/chrono.h>
 #include <fstream>
 #include <future>
 

 PYBIND11_MODULE(pyterse, m)
 {
     m.doc() = "Python bindings for Terse";
     
     using namespace jpa;
     namespace py = pybind11;
 
     /**
      * @class python_streambuf
      * @brief Custom stream buffer implementation that wraps Python file-like objects
      *
      * This class allows C++ streams to interact with Python file-like objects by
      * implementing the necessary streambuf interface methods.
      */
     class python_streambuf : public std::streambuf {
     public:
         /**
          * @brief Construct a new python_streambuf with a Python stream
          * @param py_stream Python file-like object with read/write methods
          */
         explicit python_streambuf(py::object py_stream) : py_stream(std::move(py_stream)), buffer(1024) {}
         
     protected:
         /**
          * @brief Write characters to the underlying Python stream
          * @param s Pointer to the character buffer
          * @param n Number of characters to write
          * @return Number of characters successfully written
          * @throws Various exceptions if writing fails
          */
         std::streamsize xsputn(char const* s, std::streamsize const n) override {
             try {
                 py_stream.attr("write")(py::bytes(s, static_cast<size_t>(n)));
                 return n;  
             }
             catch (const py::error_already_set& e) { throw; }
             catch (const std::exception& e) { throw py::value_error(std::string("[xsputn] Error: ") + e.what()); }
             catch (...) { throw std::runtime_error("[xsputn] Unknown error occurred"); }
         }
         
         /**
          * @brief Read characters from the underlying Python stream
          * @param s Pointer to the destination buffer
          * @param n Maximum number of characters to read
          * @return Number of characters successfully read
          * @throws Various exceptions if reading fails
          */
         std::streamsize xsgetn(char* s, std::streamsize const n) override {
             if (n == 0) return 0;
             try {
                 py::bytes data = py_stream.attr("read")(n);
                 std::string str_data = data;
                 if (str_data.empty()) throw py::value_error("[xsgetn] No data read; stream may be at EOF.");
                 std::streamsize size = std::min<std::streamsize>(n, str_data.size());
                 std::copy(str_data.begin(), str_data.begin() + size, s);
                 return size;
             }
             catch (const py::error_already_set& e) { throw; }
             catch (const std::exception& e) { throw py::value_error(std::string("[xsgetn] Error: ") + e.what()); }
             catch (...) { throw std::runtime_error("[xsgetn] Unknown error occurred"); }
         }
         
         /**
          * @brief Seek to a position within the Python stream
          * @param off Offset to seek to
          * @param way Direction of seeking (beginning, current, end)
          * @param which I/O mode (not used)
          * @return New absolute position after seeking
          * @throws Various exceptions if seeking fails
          */
         std::streampos seekoff(std::streamoff const off, std::ios_base::seekdir const way, std::ios_base::openmode const which) override {
             try {
                 int whence = (way == std::ios_base::beg) ? 0 :
                              (way == std::ios_base::cur) ? 1 :
                              (way == std::ios_base::end) ? 2 : -1;
                 if (whence == -1) throw py::value_error("Invalid seek direction");
                 py_stream.attr("seek")(off, whence);
                 auto new_pos = py_stream.attr("tell")().cast<std::streamoff>();
                 return std::streampos(new_pos);
             }
             catch (const py::error_already_set& e) { throw; }
             catch (const std::exception& e) { throw py::value_error(std::string("[seekoff] Error: ") + e.what()); }
             catch (...) { throw std::runtime_error("[seekoff] Unknown error occurred"); }
         }
         
         /**
          * @brief Read a character when the input buffer is empty
          * @return The next character as an int, or EOF if no more characters
          */
         int underflow() override {
             char c;
             if (xsgetn(&c, 1) == 0) return traits_type::eof();
             setg(&c, &c, &c + 1);
             return traits_type::to_int_type(c);
         }
         
     private:
         py::object py_stream;  ///< Python file-like object
         std::vector<char> buffer;  ///< Internal buffer for streambuf operations
     };
     
     /**
      * @class python_ostream
      * @brief C++ output stream that wraps a Python file-like object
      */
     class python_ostream : public std::ostream {
     public:
         /**
          * @brief Construct a new python_ostream with a Python stream
          * @param py_stream Python file-like object with write method
          */
         explicit python_ostream(py::object py_stream) : std::ostream(nullptr), buffer(std::move(py_stream)) { this->init(&buffer); }
     private:
         python_streambuf buffer;  ///< Stream buffer that interfaces with Python
     };
 
     /**
      * @class python_istream
      * @brief C++ input stream that wraps a Python file-like object
      */
     class python_istream : public std::istream {
     public:
         /**
          * @brief Construct a new python_istream with a Python stream
          * @param py_stream Python file-like object with read method
          */
         explicit python_istream(py::object py_stream) : std::istream(nullptr), buffer(std::move(py_stream)) { this->init(&buffer); }
     private:
         python_streambuf buffer;  ///< Stream buffer that interfaces with Python
     };
     
     /**
      * @brief Select appropriate Terse function based on NumPy array dtype
      * @param data NumPy array to analyze
      * @param func Function to call with the determined C++ type
      * @return Result of the function call
      * @throws py::type_error if the data type is not supported
      */
     auto select_terse_func = [](py::array& data, auto&& func) {
         // Handle float types
         if (data.dtype().kind() == 'f') {
             switch (data.dtype().itemsize()) {
                 case 4: return func(float{});
                 case 8: return func(double{});
                 default:
                     throw py::type_error("Terse supports float32 and float64 types");
             }
         } 
         else if (data.dtype().kind() == 'i' || data.dtype().kind() == 'u') {
             switch (data.dtype().itemsize()) {
                 case 1: return data.dtype().kind() == 'i' ? func(std::int8_t {}) : func(std::uint8_t {});
                 case 2: return data.dtype().kind() == 'i' ? func(std::int16_t{}) : func(std::uint16_t{});
                 case 4: return data.dtype().kind() == 'i' ? func(std::int32_t{}) : func(std::uint32_t{});
                 case 8: return data.dtype().kind() == 'i' ? func(std::int64_t{}) : func(std::uint64_t{});
                 default:
                     throw py::type_error("Terse only supports compression of integral values with 1, 2, 4 or 8 bytes");
             }
         }
         else {
             throw py::type_error("Terse only supports integral and floating point types");
         }
     };
     
     /**
      * @brief Determine appropriate NumPy dtype based on Terse object properties
      * @param terse Terse object to analyze
      * @return Corresponding NumPy dtype
      */
     auto pydtype_of_terse = [&] (Terse<Concurrent>& terse) {
         if (terse.is_float()) {
             return py::dtype::of<float>(); 
         }
         switch (terse.bits_per_val()) {
             case 8 : return terse.is_signed() ? py::dtype::of<int8_t >() : py::dtype::of<uint8_t >();
             case 16: return terse.is_signed() ? py::dtype::of<int16_t>() : py::dtype::of<uint16_t>();
             case 32: return terse.is_signed() ? py::dtype::of<int32_t>() : py::dtype::of<uint32_t>();
             default: return terse.is_signed() ? py::dtype::of<int64_t>() : py::dtype::of<uint64_t>();
         }
     };
     
     /**
      * @brief Calculate the shape of a NumPy array from a Terse object
      * @param terse Terse object to analyze
      * @return Vector of dimensions for the NumPy array
      */
     auto pyshape_of_terse = [] (Terse<Concurrent>& terse) {
         if (terse.dim().empty())
             return std::vector<size_t>({terse.size()});
         if (terse.number_of_frames() == 1)
             return terse.dim();
         std::vector<size_t> shape = terse.dim();
         shape.insert(shape.begin(), terse.number_of_frames());
         return shape;
     };
     
     /**
      * @brief Insert data from a NumPy array into a Terse object
      * @tparam T C++ data type of the array elements
      * @param terse Target Terse object
      * @param pos Position to insert at
      * @param data Source NumPy array
      * @param mode Terse compression mode
      * @param type Type tag used for template deduction
      * @throws py::value_error if dimensions do not match
      */
     auto insert = [] <typename T> (Terse<Concurrent>& terse, std::size_t pos, py::array& data, Terse_mode mode, T type) {
         auto const buf = data.request();
         auto const shape = std::vector<size_t>(buf.shape.begin(), buf.shape.end());
         auto dim = std::vector<size_t>(shape.begin() + (shape.size() <= 2 ? 0 : 1), shape.end());
         if (terse.number_of_frames() == 0) terse.dim(dim);
         else if (dim != terse.dim())
             throw py::value_error("Dimension mismatch: Terse cannot insert data because of shape mismatch.");
         size_t const frame_size = std::max(static_cast<std::size_t>(shape.back()), 
             static_cast<std::size_t>(std::accumulate(dim.begin(), dim.end(), std::size_t{1}, std::multiplies<>())));
         size_t const num_frames = shape.size() <= 2 ? 1 : shape[0];
         T* base_ptr = static_cast<T*>(buf.ptr);
         for (std::size_t i = 0; i < num_frames; ++i)
             terse.insert(pos + i, std::span(base_ptr + i * frame_size, frame_size), mode);
         terse.shrink_to_fit();
     };
     
     /**
      * @brief Decompress Terse data into a NumPy array
      * @tparam T C++ data type of the array elements
      * @param terse Source Terse object
      * @param data Target NumPy array
      * @param type Type tag used for template deduction
      * @throws py::value_error if dimensions do not match
      */
     auto prolix = [&] <typename T> (Terse<Concurrent>& terse, py::array& data, T type) {
         auto const buf = data.request();
         auto const shape = std::vector<size_t>(buf.shape.begin(), buf.shape.end());
         if (terse.dim().empty()) {
             if (shape.size() != 1 || shape[0] != terse.size())
                 throw py::value_error("Dimension mismatch: Terse cannot expand data into array because of shape mismatch.");
         }
         else if (terse.number_of_frames() == 1) {
             if (terse.dim() != shape)
                 throw py::value_error("Dimension mismatch: Terse cannot expand data into array because of shape mismatch.");
         }
         else if (terse.dim() != std::vector<size_t>(shape.begin() + 1, shape.end()) && shape[0] != terse.number_of_frames())
             throw py::value_error("Dimension mismatch: Terse cannot expand data into array because of shape mismatch.");
         T* base_ptr = static_cast<T*>(buf.ptr);
         std::vector<std::span<T>> frames;
         for (std::size_t i = 0; i != terse.number_of_frames(); ++i)
             frames.push_back(std::span<T>(base_ptr + i * terse.size(), terse.size()));
         terse.prolix(frames);
     };
     
 
     /**
      * @brief Python enum for Terse compression modes
      */
     py::enum_<Terse_mode>(m, "TerseMode")
         .value("SIGNED", Terse_mode::Signed, "Signed integer mode")
         .value("UNSIGNED", Terse_mode::Unsigned, "Unsigned integer mode")
         .value("SMALL_UNSIGNED", Terse_mode::Small_unsigned, "Small unsigned integer mode")
         .value("DEFAULT", Terse_mode::Default, "Default mode based on data type")
         .export_values();
     
     /**
      * @brief Python bindings for the Terse class
      */
     py::class_<Terse<Concurrent>, std::shared_ptr<Terse<Concurrent>>>(m, "Terse")
         .def(py::init<>(), "Create an empty Terse object")
     
         .def(py::init([&](py::array data, Terse_mode mode) -> std::shared_ptr<Terse<Concurrent>> {
             auto terse_ptr = std::make_shared<Terse<Concurrent>>();
             select_terse_func(data, [&](auto Type) { return insert(*terse_ptr, 0, data, mode, Type); });
             return terse_ptr;
         }), py::arg("data"), py::arg("mode") = Terse_mode::Default,
              "Initialize a Terse object with a NumPy array and a Terse_mode")
     
         .def(py::init([](py::object py_stream) -> std::shared_ptr<Terse<Concurrent>> {
             python_istream stream(py_stream);  
             return std::make_shared<Terse<Concurrent>>(stream);  
         }), py::arg("stream"),
              "Create a Terse object from a binary input stream. The stream must contain Terse data preceded by the required XML header.")
     
         .def("insert", [&](Terse<Concurrent>& terse, std::size_t pos, py::array data, Terse_mode mode) {
             if (pos > terse.number_of_frames())
                 throw std::invalid_argument("Cannot insert at this position: the terse object has fewer frames that the position requested");
             select_terse_func(data, [&](auto Type) { return insert(terse, pos, data, mode, Type); });
         }, py::arg("pos"), py::arg("data"), py::arg("mode") = Terse_mode::Default,
              "Insert data into the Terse object at the specified position.")
     
         .def("push_back", [&](Terse<Concurrent>& terse, py::array data, Terse_mode mode) {
             select_terse_func(data, [&](auto Type) { return insert(terse, terse.number_of_frames(), data, mode, Type); });
         }, py::arg("data"), py::arg("mode") = Terse_mode::Default,
              "Append data at the end of the Terse object.")
     
         .def("prolix", [&](Terse<Concurrent>& terse, py::array& data) {
             select_terse_func(data, [&](auto Type) { return prolix(terse, data, Type); });
             return data;
         }, py::arg("data"),
              "Decompress data into the provided data array.")
     
         .def("prolix", [&](Terse<Concurrent>& terse) -> py::array {
             py::array data(pydtype_of_terse(terse), pyshape_of_terse(terse));
             select_terse_func(data, [&](auto Type) { return prolix(terse, data, Type); });
             return data;
         },
              "Decompress data into a new data array.")
     
         .def("at", [](Terse<Concurrent>& self, std::size_t pos) -> std::shared_ptr<Terse<Concurrent>> {
             if (pos >= self.number_of_frames())
                 throw py::index_error("Requested frame not present: index too high.");
             return std::make_shared<Terse<Concurrent>>(self.at(pos));
         }, py::arg("pos"),
            "Return a Terse object representing a single frame at the specified position.")
     
         .def("erase", &Terse<Concurrent>::erase, py::arg("pos"),
              "Erase the frame at the specified position.")
              
         .def("write", [](Terse<Concurrent>& self, py::object stream) {
             python_ostream out(stream);
             self.write(out);
         }, py::arg("stream"),
            "Write Terse data to a binary output stream.")
 
         .def("save", [](Terse<Concurrent>& self, const std::string& filename) {
             std::ofstream outfile(filename, std::ios::binary);
             if (!outfile.is_open()) {
                 throw std::runtime_error("Failed to open file for writing");
             }
             self.write(outfile);
             outfile.close();
         }, 
         py::arg("filename"),
         "Save Terse data to a file.")
 
         .def_static("load", [](const std::string& filename) -> std::shared_ptr<Terse<Concurrent>> {
             std::ifstream infile(filename, std::ios::binary);
             if (!infile.is_open()) {
                 throw std::runtime_error("Failed to open file for reading");
             }
             auto obj = std::make_shared<Terse<Concurrent>>(infile);
             return obj;
         }, py::arg("filename"),
         "Load Terse data from a file.")
     
         .def_property_readonly("size", &Terse<Concurrent>::size,
                               "Get the number of values in each frame.")
         .def_property_readonly("number_of_frames", &Terse<Concurrent>::number_of_frames,
                               "Get the number of frames in the Terse object.")
         .def_property_readonly("is_signed", &Terse<Concurrent>::is_signed,
                               "Check if the data is stored as signed values.")
         .def_property_readonly("bits_per_val", &Terse<Concurrent>::bits_per_val,
                               "Get the number of bits used per value in the compressed data.")
         .def_property_readonly("terse_size", &Terse<Concurrent>::terse_size,
                               "Get the size of the compressed data in bytes.")
         .def_property_readonly("file_size", &Terse<Concurrent>::file_size,
                               "Get the total file size including headers in bytes.")
         .def_property_readonly("number_of_bytes", &Terse<Concurrent>::terse_size,
                               "Alias for terse_size.")
     
         .def("metadata", py::overload_cast<std::size_t>(&Terse<Concurrent>::metadata, py::const_), py::arg("frame") = 0,
              "Get the metadata for the specified frame.")
         .def("set_metadata", py::overload_cast<std::size_t, std::string const>(&Terse<Concurrent>::metadata),
              py::arg("frame"), py::arg("metadata"),
              "Set the metadata for the specified frame.")
         .def("dim", py::overload_cast<>(&Terse<Concurrent>::dim, py::const_),
              "Get the dimensions of the frames.")
         .def("set_dim", [](Terse<Concurrent>& self, const std::vector<size_t>& dims) {
             if (!self.dim().empty() && std::accumulate(dims.begin(), dims.end(), std::size_t{1}, std::multiplies<>()) != self.size())
                 throw py::value_error("The total number of elements must remain unchanged when setting dimensions, unless the Terse object is empty.");
             self.dim(dims);
         }, py::arg("dimensions"),
            "Set the dimensions of the frames.")
         .def("fractional_precision", py::overload_cast<>(&Terse<Concurrent>::fractional_precision, py::const_),
              "Get the fractional precision for floating-point data.")
         .def("set_fractional_precision", [](Terse<Concurrent>& self, double precision) {
             self.fractional_precision(precision);
         }, py::arg("precision"),
            "Set the fractional precision for floating-point data.")
         .def("block_size", py::overload_cast<>(&Terse<Concurrent>::block_size, py::const_),
              "Get the block size used for compression.")
         .def("set_block_size", [](Terse<Concurrent>& self, std::size_t block_size) {
             if (self.number_of_frames() > 0)
                 throw py::value_error("Cannot set the block size after frames have been added to the Terse object.");
             self.block_size(block_size);
         }, py::arg("block_size"),
            "Set the block size used for compression.")
         .def("fast", py::overload_cast<>(&Terse<Concurrent>::fast, py::const_),
              "Check if fast mode is enabled.")
         .def("set_fast", py::overload_cast<bool>(&Terse<Concurrent>::fast), py::arg("value"),
              "Enable or disable fast mode.")
         .def("small", py::overload_cast<>(&Terse<Concurrent>::small, py::const_),
              "Check if small mode is enabled.")
         .def("set_small", py::overload_cast<bool>(&Terse<Concurrent>::small), py::arg("value"),
              "Enable or disable small mode.")
         .def("dop", py::overload_cast<>(&Terse<Concurrent>::dop, py::const_),
              "Get the degree of parallelism.")
         .def("set_dop", py::overload_cast<double>(&Terse<Concurrent>::dop), py::arg("value"),
              "Set the degree of parallelism.")
         .def("shrink_to_fit", &Terse<Concurrent>::shrink_to_fit,
              "Reduce memory usage by freeing unused capacity.");
 }