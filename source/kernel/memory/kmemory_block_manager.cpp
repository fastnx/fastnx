#include <kernel/types.h>
#include <kernel/memory/kmemory_block_manager.h>

FastNx::Kernel::Memory::KMemoryBlockManager::KMemoryBlockManager(void *region, const U64 size) {
    const auto pages{size / SwitchPageSize};
    blocks.insert_unique(KMemoryBlock{
        .base = region,
        .pagescount = pages,
        .state = MemoryType::Free
    });
}
