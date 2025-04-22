#pragma once

#include <unordered_map>
#include <common/types.h>
namespace FastNx::Kernel::Memory {
    enum class MemoryType : U8 {
        Free
    };

    struct MemoryState {
        MemoryType type: 7;
        bool canreprotec;
    };
    struct KMemoryBlock {
        U64 pagescount;
        MemoryState state{};
        U16 ipcrefs{};
        U16 devicerefs{};
        U8 permission{};
    };

    class KMemoryBlockManager {
    public:
        KMemoryBlockManager() = default;

        void Initialize(const std::span<U8> &_blockfd);

        std::unordered_map<U8 *, KMemoryBlock> treemap;
        std::span<U8> blockfd;
    };
}