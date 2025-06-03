#include <boost/align/align_down.hpp>
#include <boost/align/align_up.hpp>

#include <kernel/types/kprocess.h>
#include <jit/page_table.h>


namespace FastNx::Jit {
    PageTable::PageTable(const std::shared_ptr<Kernel::Types::KProcess> &process) {
        const auto &memory{process->memory};
        CreateTable(TableType::Code, memory->code.begin().base(), memory->code.size());
        CreateTable(TableType::Stack, memory->stack.begin().base(), process->stacksize);
    }
    U8 *PageTable::GetTable(const void *useraddr) const {
        const auto *aligned{boost::alignment::align_down(const_cast<void *>(useraddr), Kernel::SwitchPageSize)};
        const auto offset{reinterpret_cast<U64>(useraddr) - reinterpret_cast<U64>(aligned)};

        return static_cast<U8 *>(aligned ? table[reinterpret_cast<U64>(aligned) / Kernel::SwitchPageSize] : table.front()) + offset;
    }

    U64 PageTable::GetPage(const void *begin) const {
        return std::distance(table.begin(), std::ranges::find(table, begin)) * Kernel::SwitchPageSize;
    }

    void PageTable::CreateTable(const TableType type, void *begin, const U64 size) {
        auto *bytesit{static_cast<U8 *>(begin)};
        const auto *endit{static_cast<U8 *>(begin) + size};

        table.reserve((endit - bytesit) / Kernel::SwitchPageSize);
        while (bytesit != endit) {
            table.emplace_back(bytesit);
            bytesit += Kernel::SwitchPageSize;
        }

        tableinfo.emplace(type, std::make_pair(GetPage(begin), GetPage(bytesit)));
    }

    TableType PageTable::Contains(void *usertable, U64 size) const {
        usertable = static_cast<U8 *>(boost::alignment::align_down(usertable, Kernel::SwitchPageSize));
        auto type{TableType::None};

        size = boost::alignment::align_up(size, Kernel::SwitchPageSize);
        const auto tableindex{usertable ? reinterpret_cast<U64>(usertable) / Kernel::SwitchPageSize : 0ULL};

        const auto pages{size / Kernel::SwitchPageSize};
        if (table.size() > tableindex) {
            auto infoit{tableinfo.cbegin()};
            for (; infoit != tableinfo.cend() && type == TableType::None; ++infoit)
                if (const auto tablenum{reinterpret_cast<U64>(usertable)}; infoit->second.first >= tablenum && tablenum + pages < infoit->second.second)
                    type = infoit->first;
            if (type == TableType::None && table.size() - tableindex >= pages)
                type = std::prev(tableinfo.cend())->first;
        }
        return type;
    }
}
