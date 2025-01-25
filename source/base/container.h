#pragma once

#include <array>

namespace FastNx {
    class FsPath;

    template<typename... T>
    auto ArrayOf(T &&... list) {
        return std::array{std::forward<T>(list)...};
    }

    template<class T>
    auto IsEmpty(const T &container) {
        return container == T{};
    }

    template<typename T>
    auto GetStr(const T &container) {
        if constexpr (std::is_same_v<T, FsPath>) {
            return container.c_str();
        }
        return container.c_str();
    }
}
