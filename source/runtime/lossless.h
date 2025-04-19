#pragma once

#include <common/types.h>
namespace FastNx::Runtime {
    U64 FastLz4(const std::span<char> &dest, const std::span<const char> &source);

    template <typename T>
    U64 FastLz4(const std::span<T> &dest, const std::span<T> &source) {
        if (dest.size() < source.size())
            return {};
        return FastLz4(std::span{reinterpret_cast<char*>(dest.data()), dest.size()}, std::span{reinterpret_cast<const char*>(source.data()), source.size()});
    }
}