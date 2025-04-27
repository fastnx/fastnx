#pragma once

#include <functional>
#include <map>
#include <common/types.h>
#include <common/memory.h>
#include <kernel/types.h>


namespace FastNx::Kernel::Memory {
    enum class MemoryType : U8 {
        Free,
        Code,
        Inaccessible
    };

    namespace MemoryTypeValues {
        constexpr auto Free{0U};
        constexpr auto Code{0x00DC7E03U};
        constexpr auto Inaccessible{0x00000010U};
    }

    union MemoryState {
        MemoryState() = default;
        // ReSharper disable once CppNonExplicitConvertingConstructor
        MemoryState(const U32 type) : _type(type) {}

        struct {
            MemoryType type: 7;
            bool canreprotec: 1;
            bool candebug: 1;
            bool canuseipc: 1;
            bool canusenondeviceipc: 1;
            bool canusenonsecureipc: 1;
            bool mapped: 1;
            bool code: 1;
            U32 unused: 11;
        };
        U32 _type;
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

        void Initialize(std::span<U8> &addrspace, U64 assize, const Kernel &kernel);

        void Map(const std::pair<U8 *, KMemoryBlock> &allocate);
        void ForEach(const std::pair<U8 *, KMemoryBlock> &blockdesc, std::function<void(KMemoryBlock&)> &&callback);
        __attribute__((always_inline)) bool IsMappedInRange(U8 *begin, U8 *end) {
            auto first{FindBlock(begin)};
            const auto last{FindBlock(end)};
            if (first && last)
                for (; *first != *last; ++*first)
                    return true;
            return {};
        }

        std::optional<KMemoryBlock *> FindBlock(U8 *guestptr);

        MemoryBackingPtr allocator;
        std::map<U8 *, KMemoryBlock> treemap;
    };
}
