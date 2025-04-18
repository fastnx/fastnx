#pragma once

#include <list>
#include <common/types.h>

namespace FastNx::Kernel::Memory {
    // 5.0.0
    class KSlabHeap {
    public:
        KSlabHeap(void *region, U64 size, U64 itemsize);

        void *Allocate();
        void Free(void *pointer);
    private:
        void *begin{nullptr};
        void *end{nullptr};
        std::list<void *> objects;
    };
}
