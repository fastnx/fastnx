#pragma once
#include <cassert>
#include <cstring>
#include <string_view>
#include <utility>

namespace FastNx {
    template <typename T> requires (std::is_unsigned_v<T>)
    constexpr auto ConstMagicValue(const std::string_view &magic) {
        assert(!magic.empty());
        T value{};
        std::memcpy(&value, magic.data(), std::min(magic.size(), sizeof(T)));
        if (value)
            return value;

        std::unreachable();
    }

    U64 CalculateCoverage(auto pick, auto total) {
        return static_cast<F64>(pick) / static_cast<F64>(total) * 100;
    }
}
