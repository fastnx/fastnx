#pragma once
#include <utility>
#include <array>
#include <algorithm>

#include <common/traits.h>
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
    auto GetDataArray(const T &value) {
        if constexpr (IsVectorType<T> || IsArrayType<T>) {
            return value.data();
        } else if constexpr (IsStringType<T>) {
            return value.data();
        } else if constexpr (std::is_base_of_v<T, FsSys::FsPath>) {
            return value.c_str();
        }
        std::unreachable();
    }

    template<typename T> requires (IsVectorType<T>)
    auto SizeofVector(const T &vector) {
        return sizeof(vector[0]) * vector.size();
    }

    constexpr auto IsEqual(const auto &first, const auto &second) {
        return std::ranges::equal(first, second);
    }
    auto ToSpan(const auto &container) {
        return std::span(container.begin(), container.begin());
    }

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
        std::span<const T> _values(values);
        return Contains(container, _values);
    }
    constexpr auto Contains(const auto &first, const auto &second) {
        return std::ranges::contains(first, second);
    }

    template<typename Dest, typename Source>
    U64 Copy(Dest &dest, const Source &source) {
        NX_ASSERT(source.size() <= dest.size()); // The source container is smaller than the destination container

        if constexpr (IsFlatArray<Source> && IsFlatArray<Dest>) {
            std::memcpy(dest.data(), source.data(), dest.size());
            return dest.size();
        } else {
            U64 count{};
            for (const auto &[index, _]: std::views::enumerate(dest)) {
                dest[index] = source[index];
                count++;
            }
            return count;
        }
    }

    template<typename T>
    auto EraseAllWith(std::vector<T> &values, const T value = {}) {
        for (auto it{values.begin()}; it != values.end(); ) {
            if (*it == value)
                it = values.erase(it);
            else ++it;
        }
        return values;
    }
}
