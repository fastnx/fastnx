#pragma once

#include <cstdint>
#include <utility>
#include <ranges>
namespace FastNx {
    using U8 = std::uint8_t;
    using U16 = std::uint16_t;

    using I32 = std::int32_t;
    using U32 = std::uint32_t;
    using U64 = std::uint64_t;
    using F64 = double;

    constexpr auto EnumRange(const auto front, const auto back) {
        return std::views::iota(std::to_underlying(front), std::to_underlying(back) + 1) | std::views::transform([](std::underlying_type_t<decltype(front)> _val) -> decltype(auto) { return static_cast<decltype(front)>(_val); });
    }
    constexpr U64 operator"" _KBYTES(const long long unsigned size) {
        return size * 1024;
    }
    constexpr U64 operator"" _MBYTES(const long long unsigned size) {
        return size * 1024 * 1024;
    }
}
