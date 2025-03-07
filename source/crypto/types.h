#pragma once
#include <fs_sys/types.h>
namespace FastNx::Crypto {
    using Rsa2048 = std::array<U8, 0x100>;
    using RightsId = std::array<U8, 0x10>;

    template<U64 Bits>
    struct SourceKey : std::array<U8, Bits / 8> {};

    using Key128 = SourceKey<128>;
    using Key256 = SourceKey<256>;

    bool CheckNcaIntegrity(const FsSys::VfsBackingFilePtr &file);
}
