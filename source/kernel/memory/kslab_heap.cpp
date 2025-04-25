#include <common/exception.h>
#include <kernel/memory/kslab_heap.h>


namespace FastNx::Kernel::Memory {
    KSlabHeap::KSlabHeap(const std::span<U8> &slabbase, const U64 itemsize) : slabmem(slabbase) {
        auto *begin{slabmem.begin().base()};
        const auto *end{slabmem.end().base()};
        for (auto *itemlist{begin}; itemlist < end; itemlist += itemsize) {
            objects.emplace_back(itemlist);
        }
        objects.reverse();
    }

    void *KSlabHeap::Allocate() {
        if (objects.empty())
            return nullptr;

        auto *object{objects.front()};
        if (object)
            objects.pop_front();
        return object;
    }
    void KSlabHeap::Free(void *pointer) {
        if (slabmem.begin().base() >= pointer && pointer < slabmem.end().base())
            objects.emplace_front(pointer);
        else
            throw exception{"This object does not belong to this slab"};
    }
}
