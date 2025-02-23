#pragma once

#include <vector>

#include <common/types.h>
namespace FastNx {
    template <typename T, U64 Size>
    concept IsSizeMatch = sizeof(T) == Size && std::is_trivial_v<T>;

    // ReSharper disable once CppTemplateParameterNeverUsed
    template<typename T>
    struct is_vector : std::false_type {
    };
    template<typename T, typename A>
    struct is_vector<std::vector<T, A> > : std::true_type {
    };
    template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;
}
