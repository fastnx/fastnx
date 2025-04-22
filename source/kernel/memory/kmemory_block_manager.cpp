#include <kernel/types.h>
#include <kernel/memory/kmemory_block_manager.h>

void FastNx::Kernel::Memory::KMemoryBlockManager::Initialize(const std::span<U8> &_blockfd) {
    if (_blockfd.empty())
        return;
    blockfd = _blockfd;

    const auto count{static_cast<U64>(blockfd.end() - blockfd.begin()) / SwitchPageSize};
    KMemoryBlock block{
        .pagescount = count,
        .state = MemoryType::Free
    };
    treemap.insert_or_assign(blockfd.begin().base(), block);
}
