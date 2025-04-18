#include <common/exception.h>
#include <kernel/memory/kslab_heap.h>


namespace FastNx::Kernel::Memory {
    KSlabHeap::KSlabHeap(void *region, const U64 size, const U64 itemsize) : begin(region), end(region + size) {
        for (void *itemlist{region}; itemlist < end; itemlist += itemsize) {
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
        if (begin >= pointer && pointer < end)
            objects.emplace_front(pointer);
        else
            throw exception{"This object does not belong to this slab"};
    }
}
