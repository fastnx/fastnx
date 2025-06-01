#pragma once
#include <fmt/format.h>
#include <common/types.h>

namespace FastNx {

    constexpr U64 StringsCounter{128};
    fmt::memory_buffer Strings(const void *begin, U64 size, U64 strings = StringsCounter);

    // Search for interesting topics
    void PrintTopics(const std::string_view &strings, const FsSys::FsPath &nsoname);
}
