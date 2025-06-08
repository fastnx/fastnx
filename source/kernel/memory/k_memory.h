#pragma once
#include <jit/page_table.h>
#include <kernel/memory/kmemory_block_manager.h>
#include <kernel/svc/types.h>


namespace FastNx::Kernel::Memory {
    enum class SegmentType {
        As39,
        Code,
        Alias,
        Heap,
        Stack,
        TlsIo,
        Aslr
    };

    struct MemoryAttribute {
        unsigned locked: 1;
        unsigned ipclocked: 1;
        unsigned devicedshared: 1;
        unsigned uncached: 1;
        unsigned permlocked: 1;
    };
    struct MemoryInfo {
        const void *base;
        U64 size;
        U32 type;
        MemoryAttribute attribute;
        U32 permission;
        U32 ipcrefcount;
        U32 devrefcount;
        U32 padding;
    };
    static_assert(sizeof(MemoryInfo) == 0x24 + 4);

    class KMemory {
    public:
        explicit KMemory(Kernel &_kernel);
        void InitializeForProcess(const std::shared_ptr<Types::KProcess> &process, const Svc::CreateProcessParameter &proccfg);

        void MapCodeMemory(U64 begin, U64 size, const std::vector<U8> &content) const;
        void MapTlsMemory(U64 begin, U64 size) const;
        void MapStackMemory(U64 begin, U64 size) const;

        void SetMemoryPermission(U64 begin, U64 size, I32 permission) const;
        void FillMemory(U64 begin, U8 constant, U64 size) const;

        auto *GetSegment(const SegmentType type) {
            if (type == SegmentType::Code)
                return &code;
            if (type == SegmentType::Alias)
                return &alias;
            if (type == SegmentType::Heap)
                return &heap;
            if (type == SegmentType::Stack)
                return &stack;
            if (type == SegmentType::TlsIo)
                return &tlsio;
            std::unreachable();
        }

        std::optional<MemoryInfo> QueryMemory(const U8 *begin) const;

        std::span<U8> addrspace;
        std::span<U8> code, alias, heap, stack, tlsio;

        U64 processwidth{};
    private:
        void MapSegmentMemory(const std::span<U8> &segment, U64 begin, U64 size, bool fill, const KMemoryBlock &block) const;

        Kernel &kernel;
        std::shared_ptr<KMemoryBlockManager> blockslist;
        std::shared_ptr<Jit::PageTable> table;
    };

}
