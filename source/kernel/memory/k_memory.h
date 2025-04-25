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

    class KMemory {
    public:
        explicit KMemory(Kernel &_kernel);
        void InitializeProcessMemory(const Svc::CreateProcessParameter &proccfg);

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

        std::span<U8> addrspace;
        std::span<U8> code;
        std::span<U8> alias;
        std::span<U8> heap;
        std::span<U8> stack;
        std::span<U8> tlsio;

        KMemoryBlockManager ptblocks;
        Kernel &kernel;
    };

}
