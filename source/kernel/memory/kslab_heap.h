#pragma once

#include <list>
#include <common/types.h>
#include <kernel/types.h>


namespace FastNx::Kernel::Memory {
    // 5.0.0
    class KSlabHeap {
    public:
        KSlabHeap(const MemoryDescriptor &region, U64 itemsize);

        void *Allocate();
        void Free(void *pointer);
    private:
        const MemoryDescriptor region;
        std::list<void *> objects;
    };
}
