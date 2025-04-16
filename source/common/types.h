#pragma once

#include <cstdint>
#include <limits>
#include <utility>
#include <ranges>
#include <boost/assert.hpp>

#define NX_ASSERT BOOST_VERIFY

static_assert(sizeof(void*) == sizeof(size_t));
static_assert(sizeof(std::uintptr_t) == sizeof(void*));

static_assert(std::endian::native == std::endian::little);

static_assert(std::numeric_limits<float>::is_iec559);
static_assert(std::numeric_limits<double>::is_iec559);
static_assert(sizeof(double) > sizeof(float));
static_assert(sizeof(int) == sizeof(float));

namespace FastNx {
    using U8 = std::uint8_t;
    using U16 = std::uint16_t;

    using I32 = std::int32_t;
    using U32 = std::uint32_t;
    using U64 = std::uint64_t;
    using F64 = double;

    constexpr std::string_view process{"fastnx"};

    constexpr auto EnumRange(const auto front, const auto back) {
        using Type = std::underlying_type_t<decltype(front)>;
        const auto first{static_cast<Type>(std::to_underlying(front))};
        const auto last{static_cast<Type>(std::to_underlying(back) + 1)};

        return std::views::iota(first, last) | std::views::transform([](typeof(first) eval) -> decltype(auto) {
            return static_cast<decltype(front)>(eval);
        });
    }
    constexpr U64 operator"" _KBYTES(const long long unsigned size) {
        NX_ASSERT(size);
        return size * 1024;
    }
    constexpr U64 operator"" _MBYTES(const long long unsigned size) {
        return size * 1024_KBYTES;
    }
    constexpr U64 operator"" _GBYTES(const long long unsigned size) {
        return size * 1024_MBYTES;
    }
}
