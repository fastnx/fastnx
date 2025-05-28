#pragma once
#include <fmt/format.h>
#include <common/types.h>

namespace FastNx {

    constexpr auto StringsCounter{64};
    fmt::memory_buffer Strings(const void *begin, U64 size, U64 strings = StringsCounter);

    // Search for interesting topics
    void Sfit(const std::string_view &strings);
}
