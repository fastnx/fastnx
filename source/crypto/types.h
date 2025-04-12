#pragma once
#include <boost/container_hash/hash.hpp>

#include <fs_sys/types.h>
namespace FastNx::Crypto {
    using Rsa2048 = std::array<U8, 0x100>;
    using RightsId = std::array<U8, 0x10>;

    template<U64 Bits>
    struct SourceKey : std::array<U8, Bits / 8> {};

    template<typename T, U64 Size>
    struct ArrayHash {
        U64 operator()(const std::array<T, Size> &key) const {
            U64 result{};
            for (const auto &value: key)
                boost::hash_combine(result, value);
            return result;
        }
    };

    using Key128 = SourceKey<128>;
    using Key256 = SourceKey<256>;

    bool CheckNcaIntegrity(const FsSys::VfsBackingFilePtr &file);
    bool VerifyNcaSignature(const void *content, U64 size, const Rsa2048 &signature);
}
