#include <common/exception.h>
#include <kernel/memory/kslab_heap.h>


namespace FastNx::Kernel::Memory {
    KSlabHeap::KSlabHeap(const MemoryDescriptor &region, const U64 itemsize) : region(region) {
        auto *begin{static_cast<U8 *>(region.begin)};
        const auto *end{static_cast<U8 *>(region.end)};
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
        if (region.begin >= pointer && pointer < region.end)
            objects.emplace_front(pointer);
        else
            throw exception{"This object does not belong to this slab"};
    }
}
