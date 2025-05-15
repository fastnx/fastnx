#include <kernel/types/khandle_table.h>

namespace FastNx::Kernel::Types {
    KHandleTable::KHandleTable() {
        itemstable.resize(Size);
        freelist.reserve(Size);
        for (auto listhead{itemstable.begin()}; listhead != itemstable.end(); ++listhead) {
            listhead->listindex = std::distance(itemstable.begin(), listhead);
            freelist.push_back(listhead);
        }
    }

    U32 KHandleTable::Allocate(const std::shared_ptr<KAutoObject> &object) {
        if (!object)
            return {};

        std::lock_guard lock{tablem};
        const auto validcnt{counter++};
        object->IncreaseLifetime();

        const auto listhead{freelist.back()};
        listhead->object = object;
        listhead->handle = validcnt;
        const auto tableindex{listhead->listindex};

        freelist.pop_back();
        return validcnt << 15 | tableindex;
    }

    void KHandleTable::Free(const U32 handle) {
        const auto index{handle & 0x7FFF};
        auto table{itemstable.begin()};
        while (table != itemstable.end() && table->handle != index)
            ++table;
        if (table == itemstable.end())
            return;

        auto &&object{std::move(table->object)};
        object->DeteriorateLifetime();
        table->handle = {};

        freelist.push_back(table);
    }

    void KHandleTable::CloseAll() {
        std::lock_guard lock{tablem};
        for (auto tableit{itemstable.begin()}; tableit != itemstable.end(); ++tableit) {
            bool allocated{true};
            for (auto freeit{freelist.begin()}; freeit != freelist.end(); ++freeit)
                if (tableit == *freeit)
                    allocated = {};

            if (!allocated)
                continue;
            if (tableit->object)
                tableit->object->DeteriorateLifetime();
            freelist.emplace_back(tableit);
        }
    }
}
