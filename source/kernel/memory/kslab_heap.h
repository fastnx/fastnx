#pragma once

#include <list>
#include <common/types.h>


namespace FastNx::Kernel::Memory {
    // 5.0.0
    class KSlabHeap {
    public:
        KSlabHeap() = default;
        KSlabHeap(const std::span<U8> &slabbase, U64 itemsize);

        void *Allocate();
        void *Reserve(void *pointer, U64 size, bool fixed = false);
        void Free(void *pointer);
    private:
        const std::span<U8> slabmem;
        std::list<void *> objects;
    };
}
