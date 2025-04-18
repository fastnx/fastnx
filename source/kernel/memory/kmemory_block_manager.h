#pragma once

#include <boost/intrusive/rbtree.hpp>
#include <common/types.h>
namespace FastNx::Kernel::Memory {

    enum class MemoryType : U8 {
        Free
    };
    struct alignas(32) MemoryState {
        MemoryType type : 7;
        bool canreprotec;
    };
#pragma pack(push, 1)
    struct KMemoryBlock {
        void *base{nullptr};
        U64 pagescount;
        MemoryState state;
        U16 ipcrefs;
        U16 devicerefs;
        U8 permission;
    };
#pragma pack(pop)

    class KMemoryBlockManager {
    public:
        KMemoryBlockManager(void *region, U64 size);

        boost::intrusive::rbtree<KMemoryBlock> blocks;
        void *begin{nullptr};
        void *end{nullptr};
    };
}