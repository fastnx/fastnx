#include <boost/align/align_down.hpp>

#include <jit/page_table.h>
namespace FastNx::Jit {
    PageTable::PageTable(const std::shared_ptr<Kernel::Memory::KMemory> &memory) {
        CreateTable(memory->code.begin().base(), memory->code.size());
    }
    U8 *PageTable::GetTable(const void *useraddr) const {
        const auto *aligned{boost::alignment::align_down(const_cast<void *>(useraddr), Kernel::SwitchPageSize)};
        const auto offset{reinterpret_cast<U64>(useraddr) - reinterpret_cast<U64>(aligned)};

        return static_cast<U8 *>(aligned ? table[reinterpret_cast<U64>(aligned) / Kernel::SwitchPageSize] : table.front()) + offset;
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
}
