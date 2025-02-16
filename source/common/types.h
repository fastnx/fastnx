#pragma once

#include <cstdint>
#include <algorithm>
#include <utility>
#include <ranges>
#include <span>

namespace FastNx {
    using U8 = std::uint8_t;

    using I32 = std::int32_t;
    using U32 = std::uint32_t;
    using U64 = std::uint64_t;

    template<typename T>
    bool Contains(const auto &container, const std::span<T> &values) {
        U64 count{};
        for (const auto &value: values) {
            if (std::ranges::contains(container, value))
                count++;
        }
        return count == values.size();
    }


    template<typename T>
    bool Contains(const auto &container, const std::initializer_list<T> &values) {
        std::span<const T> valuesCopy(values);
        return Contains(container, valuesCopy);
    }

    constexpr auto EnumRange(const auto front, const auto back) {
        return std::views::iota(std::to_underlying(front), std::to_underlying(back) + 1) | std::views::transform([](std::underlying_type_t<decltype(front)> _val) -> decltype(auto) { return static_cast<decltype(front)>(_val); });
    };
}
