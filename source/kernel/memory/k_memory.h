#pragma once
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
        void InitializeProcessMemory(const Svc::CreateProcessParameter &proccfg);

        void MapCodeMemory(U64 begin, U64 size, const std::vector<U8> &content);
        void MapTlsMemory(U64 begin, U64 size);
        void MapStackMemory(U64 begin, U64 size);

        void SetMemoryPermission(U64 begin, U64 size, I32 permission);
        void FillMemory(U64 begin, U8 constant, U64 size);

        auto *GetSegment(const SegmentType type) {
            switch (type) {
                case SegmentType::Code:
                    return &code;
                case SegmentType::Alias:
                    return &alias;
                case SegmentType::Heap:
                    return &heap;
                case SegmentType::Stack:
                    return &stack;
                case SegmentType::TlsIo:
                    return &tlsio;
                default:
                    std::unreachable();
            }
        }

        std::optional<MemoryInfo> QueryMemory(const U8 *begin);

        std::span<U8> addrspace;
        std::span<U8> code;
        std::span<U8> alias;
        std::span<U8> heap;
        std::span<U8> stack;
        std::span<U8> tlsio;
    private:
        void MapSegmentMemory(const std::span<U8> &memseg, U64 begin, U64 size, bool fill, const KMemoryBlock &block);

        Kernel &kernel;
        std::optional<KMemoryBlockManager> blockslist;
    };

}
