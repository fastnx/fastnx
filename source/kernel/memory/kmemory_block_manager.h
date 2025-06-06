#pragma once

#include <functional>
#include <map>
#include <common/types.h>
#include <common/memory.h>
#include <kernel/types.h>
#include <kernel/memory/kslab_heap.h>


namespace FastNx::Kernel::Memory {
    enum class MemoryType : U8 {
        Free,
        Io,
        Static,
        Code,
        CodeData,
        Normal,
        Shared,
        Alias,
        AliasCode,
        AliasCodeData,
        Ipc,
        Stack,
        ThreadLocal,
        Transfered,
        SharedTransfered,
        SharedCode,
        Inaccessible,
        NonSecureIpc,
        NonDeviceIpc,
        Kernel,
        GeneratedCode,
        CodeOut,
        Coverage, // [13.0.0+] Coverage
        Insecure // [15.0.0+] Insecure
    };

    namespace MemoryTypeValues {
        constexpr auto Free{0U};
        constexpr auto Code{0x00DC7E03U};
        constexpr auto Inaccessible{0x00000010U};
        constexpr auto ThreadLocal{0x0040200C};
        constexpr auto Stack{0x005C3C0B};
    }

    union MemoryState {
        MemoryState() = default;
        explicit MemoryState(const U32 type) : _type(type) {}

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

        bool operator==(const MemoryState &state) const {
            return _type == state._type;
        }
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
        explicit KMemoryBlockManager(Kernel &kernel);

        std::span<U8> Initialize(U64 width, U64 assize, const Kernel &kernel);

        void Map(const std::pair<U8 *, KMemoryBlock> &allocate);
        void ForEach(const std::pair<U8 *, KMemoryBlock> &blockdesc, std::function<void(KMemoryBlock&)> &&callback);
        void Reprotect(const std::pair<U8 *, KMemoryBlock> &reprotect);
        __attribute__((always_inline)) bool IsMappedInRange(const U8 *begin, const U8 *end) {
            auto first{FindBlock(begin)};
            const auto last{FindBlock(end)};
            for (; first && last && *first != *last; first = FindBlock(begin)) {
                if (first->second->state == MemoryState{MemoryTypeValues::Free})
                    return {};
                begin += first->second->pagescount * SwitchPageSize;
            }
            return first && last;
        }

        std::optional<std::pair<const U8 *, KMemoryBlock *>> FindBlock(const U8 *guestptr);

        std::map<const U8 *, KMemoryBlock> treemap;
        const std::shared_ptr<KSlabHeap> &hostslab;
        MemoryBackingPtr allocator;
    };
}
