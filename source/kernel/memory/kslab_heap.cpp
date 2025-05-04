#include <common/exception.h>
#include <kernel/types.h>
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

    void *KSlabHeap::Reserve(void *pointer, const U64 size, const bool fixed) {
        if (!fixed && !objects.empty())
            pointer = objects.front();
        else if (fixed && std::ranges::find(objects, pointer) == objects.end())
            return MemoryFailValue;

        const auto itemsize = [&] -> U64 {
            NX_ASSERT(objects.size() >= 2);
            const auto firstone{objects.cbegin()};
            return static_cast<U8 *>(*firstone) - static_cast<U8 *>(*std::next(firstone));
        }();
        if (!itemsize)
            return MemoryFailValue;

        for (U64 offset{}; offset < size; offset += itemsize) {
            if (const auto dropitem{std::ranges::find(objects, pointer)}; dropitem != objects.end())
                objects.erase(dropitem);
            else throw exception{"The address {} has already been reserved or does not belong to this slab", fmt::ptr(pointer)};

            pointer = objects.front();
        }
        return pointer;
    }

    void KSlabHeap::Free(void *pointer) {
        if (slabmem.begin().base() >= pointer && pointer < slabmem.end().base())
            objects.emplace_front(pointer);
        else
            throw exception{"This object does not belong to this slab"};
    }
}
