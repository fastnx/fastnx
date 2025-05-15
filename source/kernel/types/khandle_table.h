#pragma once

#include <memory>
#include <kernel/kauto_object.h>


namespace FastNx::Kernel::Types {
    struct KHandleEntry {
        U16 listindex;
        U16 handle;
        std::shared_ptr<KAutoObject> object;
    };

    // 1.0.0
    class KHandleTable {
    public:
        KHandleTable();
        U32 Allocate(const std::shared_ptr<KAutoObject> &object);
        void Free(U32 handle);
        void CloseAll();
        auto GetActiveSlots() const {
            return itemstable.size() - freelist.size();
        }

        template<typename T>
        std::shared_ptr<T> GetObject(const U32 handle) {
            const auto listindex{handle & 0x7FFF};
            const auto handleid{handle >> 15};

            std::lock_guard lock{tablem};
            auto entryit{itemstable.begin()};
            while (entryit != itemstable.end() && entryit->listindex != listindex)
                ++entryit;

            if (entryit->handle != handleid)
                return nullptr;
            if constexpr (std::is_same_v<T, KThread>)
                if (entryit->object->type != KAutoType::KThread)
                    return nullptr;
            return std::dynamic_pointer_cast<T>(entryit->object);
        }

    private:

        static constexpr U16 Size{SwitchPageSize / 4};
        std::list<KHandleEntry> itemstable;

        using ListIterator = decltype(itemstable)::iterator;
        std::vector<ListIterator> freelist;

        U16 counter{1};
        std::mutex tablem;
    };
}
