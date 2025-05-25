#include <boost/align/align_down.hpp>
#include <boost/align/align_up.hpp>

#include <kernel/types/kprocess.h>
#include <jit/page_table.h>


namespace FastNx::Jit {
    PageTable::PageTable(const std::shared_ptr<Kernel::Types::KProcess> &process) {
        const auto &memory{process->memory};
        CreateTable(memory->code.begin().base(), memory->code.size());
        CreateTable(memory->stack.begin().base(), process->stacksize);
    }
    U8 *PageTable::GetTable(const void *useraddr) const {
        const auto *aligned{boost::alignment::align_down(const_cast<void *>(useraddr), Kernel::SwitchPageSize)};
        const auto offset{reinterpret_cast<U64>(useraddr) - reinterpret_cast<U64>(aligned)};

        return static_cast<U8 *>(aligned ? table[reinterpret_cast<U64>(aligned) / Kernel::SwitchPageSize] : table.front()) + offset;
    }

    U64 PageTable::GetPage(const void *begin) {
        return std::distance(table.begin(), std::ranges::find(table, begin)) * Kernel::SwitchPageSize;
    }

    void PageTable::CreateTable(void *begin, const U64 size) {
        auto *bytesit{static_cast<U8 *>(begin)};
        const auto *endit{static_cast<U8 *>(begin) + size};
        table.reserve((endit - bytesit) / Kernel::SwitchPageSize);
        while (bytesit != endit) {
            table.emplace_back(bytesit);
            bytesit += Kernel::SwitchPageSize;
        }
    }

    bool PageTable::Contains(void *usertable, U64 size) const {
        usertable = static_cast<U8 *>(boost::alignment::align_down(usertable, Kernel::SwitchPageSize));

        size = boost::alignment::align_up(size, Kernel::SwitchPageSize);
        const auto begin{reinterpret_cast<U64>(usertable) / Kernel::SwitchPageSize};
        const auto end{begin + size / Kernel::SwitchPageSize};

        return end <= table.size();
    }
}
