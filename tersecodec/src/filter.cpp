//
// Written by: Senik Matinyan, 2024
//

#include "hdf5.h"
#include "H5PLextern.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <span>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "Concurrent.hpp"
#include "Terse.hpp"

#define TERSE_FILTER_ID 32029
#define TERSE_DEFAULT_CHUNK_SIZE (1 << 18)

#define TYPE_CODE_INT16 0
#define TYPE_CODE_UINT16 1
#define TYPE_CODE_INT32 2
#define TYPE_CODE_UINT32 3
#define TYPE_CODE_INT8 4
#define TYPE_CODE_UINT8 5

template<typename T>
std::size_t hdf5_buffer_to_terse(
    void** data,
    std::size_t* size,
    std::size_t chunk_size) {
    struct Fixed_ostreambuf : public std::streambuf {
        Fixed_ostreambuf(char* buffer, std::size_t size) {
            setp(buffer, buffer + size);
        }
    };

    std::span buffer(static_cast<T*>(*data), *size / sizeof(T));

    jpa::Terse<jpa::Concurrent> terse_chunks;
    jpa::Terse<jpa::Concurrent> terse_rest;

    if (chunk_size == 0) {
        terse_chunks.push_back(buffer);
    } else {
        std::size_t pos = 0;

        for (; pos + chunk_size < buffer.size(); pos += chunk_size) {
            terse_chunks.push_back(
                std::span(buffer.data() + pos, chunk_size));
        }

        terse_rest.push_back(
            std::span(buffer.data() + pos, buffer.size() - pos));
    }

    std::size_t terse_size =
        terse_chunks.file_size() + terse_rest.file_size();

    void* new_buf = malloc(terse_size);
    if (!new_buf) {
        std::cerr
            << "Allocation of memory failed while compressing data in "
               "hdf5_buffer_to_terse"
            << std::endl;
        return 0;
    }

    Fixed_ostreambuf oss_buffer(
        static_cast<char*>(new_buf),
        terse_size);
    std::ostream oss(&oss_buffer);

    terse_chunks.write(oss);
    terse_rest.write(oss);

    free(buffer.data());
    *data = new_buf;

    return *size = terse_size;
}

template<typename T>
std::size_t terse_to_hdf5_buffer(void** data, std::size_t* size) {
    struct Fixed_istreambuf : public std::streambuf {
        char* buffer;
        std::size_t buffer_size;

        Fixed_istreambuf(char* buffer, std::size_t size)
            : buffer(buffer), buffer_size(size) {
            setg(buffer, buffer, buffer + size);
        }

        std::streampos seekoff(
            std::streamoff off,
            std::ios_base::seekdir way,
            std::ios_base::openmode which =
                std::ios_base::in) override {
            switch (way) {
                case std::ios_base::beg:
                    setg(buffer, buffer + off, buffer + buffer_size);
                    break;
                case std::ios_base::end:
                    setg(
                        buffer,
                        buffer + buffer_size + off,
                        buffer + buffer_size);
                    break;
                default:
                    gbump(static_cast<int>(off));
                    break;
            }

            return gptr() - eback();
        }

        std::streampos seekpos(
            std::streampos pos,
            std::ios_base::openmode which =
                std::ios_base::in) override {
            return seekoff(pos, std::ios_base::beg, which);
        }
    };

    std::span buffer(static_cast<char*>(*data), *size);

    Fixed_istreambuf iss_buffer(
        static_cast<char*>(buffer.data()),
        buffer.size());
    std::istream iss(&iss_buffer);

    jpa::Terse<jpa::Concurrent> terse_chunks(iss);
    std::size_t chunk_size = terse_chunks.size();

    jpa::Terse<jpa::Concurrent> terse_rest;
    if (!iss.eof()) {
        terse_rest = jpa::Terse<jpa::Concurrent>(iss);
    }

    std::size_t prolix_size =
        terse_chunks.size() * terse_chunks.number_of_frames()
        + terse_rest.size();

    void* new_buf = malloc(prolix_size * sizeof(T));
    if (!new_buf) {
        std::cerr
            << "Allocation of memory failed while expanding data in "
               "terse_to_hdf5_buffer"
            << std::endl;
        return 0;
    }

    free(*data);
    *data = new_buf;

    std::span<T> prolix(
        static_cast<T*>(new_buf),
        prolix_size);

    terse_chunks.prolix(
        std::span(
            prolix.data(),
            chunk_size * terse_chunks.number_of_frames()));

    terse_rest.prolix(
        std::span(
            prolix.data()
                + chunk_size * terse_chunks.number_of_frames(),
            terse_rest.size()));

    return *size = prolix_size * sizeof(T);
}

size_t Terse_filter_cpp(
    unsigned int flags,
    size_t cd_nelmts,
    const unsigned int cd_values[],
    size_t* bufsize,
    void** buf) {
    if (!buf
        || !bufsize
        || cd_nelmts == 0
        || cd_values == nullptr
        || cd_values[0] > 5) {
        std::cerr << "Invalid arguments provided to Terse_filter"
                  << std::endl;
        return 0;
    }

    unsigned int data_type_code = cd_values[0];

    if (flags & H5Z_FLAG_REVERSE) {
        switch (data_type_code) {
            case TYPE_CODE_INT16:
                return terse_to_hdf5_buffer<int16_t>(buf, bufsize);
            case TYPE_CODE_UINT16:
                return terse_to_hdf5_buffer<uint16_t>(buf, bufsize);
            case TYPE_CODE_INT32:
                return terse_to_hdf5_buffer<int32_t>(buf, bufsize);
            case TYPE_CODE_UINT32:
                return terse_to_hdf5_buffer<uint32_t>(buf, bufsize);
            case TYPE_CODE_INT8:
                return terse_to_hdf5_buffer<int8_t>(buf, bufsize);
            case TYPE_CODE_UINT8:
                return terse_to_hdf5_buffer<uint8_t>(buf, bufsize);
        }
    } else {
        switch (data_type_code) {
            case TYPE_CODE_INT16:
                return hdf5_buffer_to_terse<int16_t>(
                    buf,
                    bufsize,
                    TERSE_DEFAULT_CHUNK_SIZE);
            case TYPE_CODE_UINT16:
                return hdf5_buffer_to_terse<uint16_t>(
                    buf,
                    bufsize,
                    TERSE_DEFAULT_CHUNK_SIZE);
            case TYPE_CODE_INT32:
                return hdf5_buffer_to_terse<int32_t>(
                    buf,
                    bufsize,
                    TERSE_DEFAULT_CHUNK_SIZE);
            case TYPE_CODE_UINT32:
                return hdf5_buffer_to_terse<uint32_t>(
                    buf,
                    bufsize,
                    TERSE_DEFAULT_CHUNK_SIZE);
            case TYPE_CODE_INT8:
                return hdf5_buffer_to_terse<int8_t>(
                    buf,
                    bufsize,
                    TERSE_DEFAULT_CHUNK_SIZE);
            case TYPE_CODE_UINT8:
                return hdf5_buffer_to_terse<uint8_t>(
                    buf,
                    bufsize,
                    TERSE_DEFAULT_CHUNK_SIZE);
        }
    }

    return 0;
}

extern "C" size_t Terse_filter(
    unsigned int flags,
    size_t cd_nelmts,
    const unsigned int cd_values[],
    size_t nbytes,
    size_t* bufsize,
    void** buf) {
    return Terse_filter_cpp(
        flags,
        cd_nelmts,
        cd_values,
        bufsize,
        buf);
}

static herr_t Terse_set_local(
    hid_t dcpl_id,
    hid_t type_id,
    hid_t space_id) {
    if (H5Tget_class(type_id) != H5T_INTEGER) {
        std::cerr
            << "Terse filter only supports integer datasets"
            << std::endl;
        return -1;
    }

    H5T_sign_t sign = H5Tget_sign(type_id);
    if (sign == H5T_SGN_ERROR) {
        return -1;
    }

    bool is_signed = sign != H5T_SGN_NONE;
    unsigned int data_type_code;

    switch (H5Tget_size(type_id)) {
        case 1:
            data_type_code =
                is_signed ? TYPE_CODE_INT8 : TYPE_CODE_UINT8;
            break;
        case 2:
            data_type_code =
                is_signed ? TYPE_CODE_INT16 : TYPE_CODE_UINT16;
            break;
        case 4:
            data_type_code =
                is_signed ? TYPE_CODE_INT32 : TYPE_CODE_UINT32;
            break;
        default:
            std::cerr
                << "Terse filter only supports 8-, 16- and 32-bit "
                   "integer datasets"
                << std::endl;
            return -1;
    }

    unsigned int flags;
    size_t cd_nelmts = 1;
    unsigned int cd_values[1] = {0};

    if (H5Pget_filter_by_id2(
            dcpl_id,
            TERSE_FILTER_ID,
            &flags,
            &cd_nelmts,
            cd_values,
            0,
            nullptr,
            nullptr) < 0) {
        return -1;
    }

    cd_values[0] = data_type_code;

    if (H5Pmodify_filter(
            dcpl_id,
            TERSE_FILTER_ID,
            flags,
            1,
            cd_values) < 0) {
        return -1;
    }

    return 1;
}

extern "C" herr_t register_terse_filter() {
    H5Z_class2_t filter_class = {
        H5Z_CLASS_T_VERS,
        TERSE_FILTER_ID,
        1,
        1,
        "TERSE",
        nullptr,
        Terse_set_local,
        Terse_filter
    };

    return H5Zregister(&filter_class);
}

extern "C" const void* H5PLget_plugin_info(void) {
    static H5Z_class2_t filter_class = {
        H5Z_CLASS_T_VERS,
        TERSE_FILTER_ID,
        H5Z_FILTER_CONFIG_ENCODE_ENABLED
            | H5Z_FILTER_CONFIG_DECODE_ENABLED,
        1,
        "TERSE",
        nullptr,
        Terse_set_local,
        Terse_filter
    };

    return static_cast<const void*>(&filter_class);
}

extern "C" H5PL_type_t H5PLget_plugin_type(void) {
    return H5PL_TYPE_FILTER;
}
