#pragma once

#include <boost/intrusive/rbtree.hpp>
#include <common/types.h>
namespace FastNx::Kernel::Memory {
    using namespace boost::intrusive;
    enum class MemoryType : U8 {
        Free
    };

    struct MemoryState {
        MemoryType type : 7;
        bool canreprotec;
    };
    struct KMemoryBlock : set_base_hook<optimize_size<true>> {
        void *base{nullptr};
        U64 pagescount;
        MemoryState state{};
        U16 ipcrefs{};
        U16 devicerefs{};
        U8 permission{};

        bool operator<(const KMemoryBlock &value) const {
            return base < value.base;
        }
    };

    class KMemoryBlockManager {
    public:
        explicit KMemoryBlockManager(const MemoryDescriptor &region);
        rbtree<KMemoryBlock> treeblocks;
        const MemoryDescriptor region;
    };
}