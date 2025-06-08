#include <boost/align/align_down.hpp>
#include <boost/align/align_up.hpp>

#include <kernel/types/kprocess.h>
#include <jit/page_table.h>


namespace FastNx::Jit {
    void PageTable::Initialize(const std::shared_ptr<Kernel::Types::KProcess> &process) {
        pagetablewidth = process->memory->processwidth;

        table.resize(1ULL << (pagetablewidth - Kernel::SwitchPageBitsCount));
        backing.resize(SwitchMemorySize / Kernel::SwitchPageSize);
    }

    auto GetOffset(const void *begin, const U64 width = 0) -> U64 {
        // ReSharper disable once CppRedundantParentheses
        return reinterpret_cast<U64>(begin) >> Kernel::SwitchPageBitsCount & ((1ULL << (width - Kernel::SwitchPageBitsCount)) - 1);
    }
    U8 *PageTable::GetTable(const void *useraddr) const {
        const auto *aligned{boost::alignment::align_down(const_cast<void *>(useraddr), Kernel::SwitchPageSize)};
        const auto offset{reinterpret_cast<U64>(useraddr) - reinterpret_cast<U64>(aligned)};

        const auto tablepage{aligned ? table[GetOffset(aligned, pagetablewidth)] : table.front()};
        return static_cast<U8 *>(tablepage(offset));
    }

    void PageTable::CreateTable(void *begin, void *host, U64 offset, const U64 size) {
        auto *beginit{static_cast<U8 *>(begin)};
        auto *hostit{static_cast<U8 *>(host) + offset};
        const auto *endit{static_cast<U8 *>(begin) + size};

        for (; beginit != endit; beginit += Kernel::SwitchPageSize, hostit += Kernel::SwitchPageSize, offset += Kernel::SwitchPageSize) {
            const auto tablestart{GetOffset(beginit, pagetablewidth)};

            table[tablestart] = Page{beginit};
            backing[offset >> Kernel::SwitchPageBitsCount] = Page{hostit};
        }

        tableinfo.emplace_back(std::make_pair(TableType::Allocated, std::make_pair(begin, beginit)));
    }
    void PageTable::DelimitTable(const TableType _type, const void *begin, const U64 size) {
        for (auto &[type, table]: tableinfo) {
            if (type == TableType::Allocated && table.first == begin &&
                table.second == static_cast<const U8 *>(begin) + size)
                type = _type;
        }
    }

    std::pair<PageAttributeType, TableType> PageTable::Contains(void *usertable, U64 size) const {
        usertable = static_cast<U8 *>(boost::alignment::align_down(usertable, Kernel::SwitchPageSize));
        auto type{TableType::Allocated};

        size = boost::alignment::align_up(size, Kernel::SwitchPageSize);
        const auto tableindex{GetOffset(usertable, pagetablewidth)};

        auto attribute{PageAttributeType::Mapped};
        for (auto livepage{tableindex}; livepage < tableindex + size / Kernel::SwitchPageSize; livepage++) {
            if (table[livepage].GetPageAttr() == PageAttributeType::Mapped)
                continue;
            attribute = PageAttributeType::Unmapped;
            break;
        }

        if (attribute == PageAttributeType::Mapped)
            for (const auto &[_type, markpage] : tableinfo)
                if (markpage.first <= usertable && static_cast<U8 *>(usertable) + size <= markpage.second)
                    type = _type;
        return std::make_pair(attribute, type);
    }
}
