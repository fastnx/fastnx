#include <kernel/types.h>
#include <kernel/memory/kmemory_block_manager.h>

FastNx::Kernel::Memory::KMemoryBlockManager::KMemoryBlockManager(const MemoryDescriptor &region) : region(region) {
    const auto count{static_cast<U64>(static_cast<U8*>(region.end) - static_cast<U8*>(region.begin)) / SwitchPageSize};
    KMemoryBlock block{
        .base = region.begin,
        .pagescount = count,
        .state = MemoryType::Free
    };
    treeblocks.insert_unique(block);
}
