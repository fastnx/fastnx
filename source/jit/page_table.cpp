#include <jit/page_table.h>
namespace FastNx::Jit {
    PageTable::PageTable(const std::shared_ptr<Kernel::Memory::KMemory> &memory) {
        CreateTable(memory->code.begin().base(), memory->code.size());
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
