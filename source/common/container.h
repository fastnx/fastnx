#pragma once
#include <utility>
#include <array>
#include <fs_sys/types.h>
namespace FastNx {
    template<typename... T>
    auto ArrayOf(T &&... list) {
        return std::array{std::forward<T>(list)...};
    }
    template<class T>
    auto IsEmpty(const T &container) {
        return container == T{};
    }

    template<typename T>
    struct is_vector : std::false_type {
    };
    template<typename T, typename A>
    struct is_vector<std::vector<T, A> > : std::true_type {
    };
    template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;

    template<typename T>
    auto LandingOf(const T &container) {
        if constexpr (is_vector_v<T>) {
            return container.data();
        }
        if constexpr (std::is_base_of_v<T, FsSys::FsPath>) {
            return container.c_str();
        }
        std::unreachable();
    }

    template<typename T> requires (is_vector_v<T>)
    auto SizeofVector(const T& vector) {
        return sizeof(vector[0]) * vector.size();
    }
}
