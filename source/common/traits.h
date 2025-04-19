// ReSharper disable CppTemplateParameterNeverUsed
#pragma once

#include <vector>
#include <array>

#include <common/types.h>
namespace FastNx {
    template<typename T>
    struct is_vector : std::false_type {};
    template<typename T, typename A>
    struct is_vector<std::vector<T, A>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_vector_v = is_vector<T>::value;

    template<typename T>
    struct is_array : std::false_type {};
    template<typename T, U64 Size>
    struct is_array<std::array<T, Size> > : std::true_type {};

    template<typename T>
    inline constexpr bool is_array_v = is_array<T>::value;

    template<class T1, class... Ts>
    constexpr bool is_one_of() noexcept {
        return (std::is_same_v<T1, Ts> || ...);
    }

    template<typename T>
    concept IsVectorType = is_vector_v<T>;
    template<typename T>
    concept IsArrayType = is_array_v<T>;

    template<typename T>
    concept IsFlatArray = IsVectorType<T> or IsArrayType<T>;

    template<typename T, U64 Size>
    concept IsSizeOf = sizeof(T) == Size and std::is_trivial_v<T>;

    template<typename T>
    concept IsStringType = is_one_of<T, std::string, std::string_view>();
}
