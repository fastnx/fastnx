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

        // ReSharper disable once CppMemberFunctionMayBeConst
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
