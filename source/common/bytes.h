#pragma once

#include <boost/algorithm/hex.hpp>

#include <common/container.h>
#include <common/types.h>

namespace FastNx {
    template<U64 Size, typename T> requires (IsStringType<T>)
    constexpr std::array<U8, Size> ToArrayOfBytes(const T &string) {
        std::array<U8, Size> result{};
        if (string.size() / 2 != result.size())
            throw std::bad_cast{};

        thread_local std::vector<U8> bytes;
        if (bytes.size() < result.size())
            bytes.reserve(result.size());
        else
            bytes.clear();

        boost::algorithm::unhex(string, std::back_inserter(bytes));
        assert(Copy(result, bytes) == bytes.size());
        return result;
    }

    template<typename T> requires (std::is_copy_assignable_v<T>)
    constexpr T ToObjectOf(const auto &string) {
        T value;
        const auto bytes{ToArrayOfBytes<sizeof(T)>(string)};
        assert(bytes.size() == sizeof(value));
        std::memcpy(&value, bytes.data(), sizeof(T));
        return value;
    }
}