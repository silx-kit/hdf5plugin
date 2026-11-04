// Builds the headers and checks a round trip through a stream, for the
// compilers of the GitHub Actions workflow (.github/workflows/build.yml).

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <sstream>
#include <type_traits>
#include <vector>

// Concurrent.hpp must be included before Terse.hpp.
#include "Concurrent.hpp"
#include "Terse.hpp"
#include "Bitqueue.hpp"
#include "Operators.hpp"
#include "Shared_array.hpp"
#include "Unique_array.hpp"
#include "XML_element.hpp"

// Terse<Concurrent> holds std::future objects and must report that it cannot be copied, otherwise pybind11
// tries to copy it and fails to compile (https://github.com/senikm/trpx2.0/issues/1).
static_assert(!std::is_copy_constructible_v<jpa::Terse<jpa::Concurrent>>);
static_assert(!std::is_copy_assignable_v<jpa::Terse<jpa::Concurrent>>);
static_assert(std::is_move_constructible_v<jpa::Terse<jpa::Concurrent>>);
static_assert(std::is_move_assignable_v<jpa::Terse<jpa::Concurrent>>);
static_assert(std::is_copy_constructible_v<jpa::Terse<>>);
static_assert(std::is_copy_assignable_v<jpa::Terse<>>);

template <typename T, typename CONCURRENT>
bool round_trip(char const* name, jpa::Terse_mode mode, char const* mode_name) {
    // Two frames of 48 values (a multiple of the block size of 12).
    std::vector<T> values(96);
    for (std::size_t i = 0; i < values.size(); ++i)
        values[i] = static_cast<T>(i % 3 == 0 ? i * 7 : i % 5);
    jpa::Terse<CONCURRENT> terse;
    terse.push_back(values.data(), 48, mode);
    terse.push_back(values.data() + 48, 48, mode);
    std::stringstream stream;
    terse.write(stream);
    jpa::Terse<CONCURRENT> read(stream);
    std::vector<T> result(values.size());
    for (std::size_t frame = 0; frame < read.number_of_frames(); ++frame)
        read.prolix(result.begin() + static_cast<std::ptrdiff_t>(frame * read.size()), frame);
    bool const ok = read.number_of_frames() == 2 && result == values;
    std::printf("%-8s %-14s %-10s %s\n", name, mode_name, std::is_same_v<CONCURRENT, void> ? "void" : "Concurrent",
                ok ? "ok" : "FAILED");
    return ok;
}

// All modes of the type, so that all encoders and decoders are built.
template <typename T, typename CONCURRENT>
bool round_trips(char const* name) {
    bool ok = round_trip<T, CONCURRENT>(name, jpa::Terse_mode::Default, "Default");
    if constexpr (std::is_signed_v<T>)
        ok &= round_trip<T, CONCURRENT>(name, jpa::Terse_mode::Signed, "Signed");
    else {
        ok &= round_trip<T, CONCURRENT>(name, jpa::Terse_mode::Unsigned, "Unsigned");
        ok &= round_trip<T, CONCURRENT>(name, jpa::Terse_mode::Small_unsigned, "Small_unsigned");
    }
    return ok;
}

template <typename CONCURRENT>
bool round_trips() {
    bool ok = true;
    ok &= round_trips<std::uint8_t, CONCURRENT>("uint8");
    ok &= round_trips<std::int8_t, CONCURRENT>("int8");
    ok &= round_trips<std::uint16_t, CONCURRENT>("uint16");
    ok &= round_trips<std::int16_t, CONCURRENT>("int16");
    ok &= round_trips<std::uint32_t, CONCURRENT>("uint32");
    ok &= round_trips<std::int32_t, CONCURRENT>("int32");
    return ok;
}

int main() {
    bool const single_threaded = round_trips<void>();
    bool const concurrent = round_trips<jpa::Concurrent>();
    return single_threaded && concurrent ? 0 : 1;
}
