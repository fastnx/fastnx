#include <kernel/kernel.h>
#include <common/exception.h>
#include <kernel/memory/kmemory_block_manager.h>



namespace FastNx::Kernel::Memory {
    void KMemoryBlockManager::Initialize(std::span<U8> &addrspace, const U64 assize, const Kernel &kernel) {
        if (!allocator)
            allocator = kernel.nxalloc;
        addrspace = kernel.nxalloc->InitializeGuestAs(assize);

        if (addrspace.empty())
            throw exception{"Could not create the AS for the current process"};

        const auto count{static_cast<U64>(addrspace.end() - addrspace.begin()) / SwitchPageSize};
        treemap.insert_or_assign(addrspace.begin().base(), KMemoryBlock{
            .pagescount = count,
            .state = MemoryTypeValues::Free
        });
        // Fix to avoid breaking the std::map algorithm
        treemap.insert_or_assign(addrspace.begin().base() + count * SwitchPageSize, KMemoryBlock{
            .pagescount = {},
            .state = MemoryTypeValues::Inaccessible
        });
    }

    void KMemoryBlockManager::Map(const std::pair<U8 *, KMemoryBlock> &allocate) {
        const auto first{treemap.lower_bound(allocate.first)};
        const auto neededpages{allocate.second.pagescount};
        const auto offset{neededpages * SwitchPageSize};

        const auto last{treemap.upper_bound(allocate.first + offset)};

        const auto ismapped{first->second.state.mapped};

        if (first == last) {
            if (!ismapped) {}

        } else if (first->first + offset < last->first) {
        } else {

        }
    }

    void KMemoryBlockManager::ForEach(const std::pair<U8 *, KMemoryBlock> &blockdesc, std::function<void(KMemoryBlock &)> &&callback) {
        const auto first{FindBlock(blockdesc.first)};
        if (!first)
            throw std::out_of_range{"Block not found"};

        auto *blockit{first.value()};
        U64 pages{blockdesc.second.pagescount};
        for (; pages; ++blockit) {
            callback(*blockit);
            if (blockit->pagescount > pages)
                break;
            pages -= blockit->pagescount;
        }
    }

    std::optional<KMemoryBlock *> KMemoryBlockManager::FindBlock(U8 *guestptr) {
        const auto firstblk{treemap.lower_bound(guestptr)};
        if (firstblk != treemap.end()) {
            if (firstblk->first == guestptr)
                return &firstblk->second;
            if (const auto size{firstblk->second.pagescount})
                if (firstblk->first >= guestptr && firstblk->first + size * SwitchPageSize <= guestptr)
                    return &firstblk->second;
        }
        return std::nullopt;
    }
}
