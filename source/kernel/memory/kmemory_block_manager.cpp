#include <kernel/types.h>
#include <kernel/memory/kmemory_block_manager.h>

void FastNx::Kernel::Memory::KMemoryBlockManager::Initialize(const std::span<U8> &addrspace) {
    if (addrspace.empty())
        return;
    basemem = addrspace;

    const auto count{static_cast<U64>(basemem.end() - basemem.begin()) / SwitchPageSize};
    KMemoryBlock block{
        .pagescount = count,
        .state = MemoryType::Free
    };
    treemap.insert_or_assign(basemem.begin().base(), block);
}
