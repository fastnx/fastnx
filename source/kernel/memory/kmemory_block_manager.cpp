#include <kernel/kernel.h>
#include <common/exception.h>
#include <kernel/memory/kmemory_block_manager.h>



namespace FastNx::Kernel::Memory {
    KMemoryBlockManager::KMemoryBlockManager(Kernel &kernel) : hostslab(kernel.poffset), allocator(kernel.systemalloc) {}

    std::span<U8> KMemoryBlockManager::Initialize(const U64 width, const U64 assize, const Kernel &kernel) {
        const auto addrspace{kernel.systemalloc->InitializeGuestAs(width, assize)};

        if (addrspace.empty())
            throw exception{"Could not create the AS for the current process"};

        const auto count{static_cast<U64>(addrspace.end() - addrspace.begin()) / SwitchPageSize};
        blocksmap.insert_or_assign(addrspace.begin().base(), KMemoryBlock{
            .pagescount = count,
            .state = MemoryState{MemoryTypeValues::Free}
        });
        // Fix to avoid breaking the std::map algorithm
        blocksmap.insert_or_assign(addrspace.begin().base() + count * SwitchPageSize, KMemoryBlock{
            .pagescount = {},
            .state = MemoryState{MemoryTypeValues::Inaccessible}
        });

        // ReSharper disable once CppDFALocalValueEscapesFunction
        return addrspace;
    }

    void KMemoryBlockManager::Map(const std::pair<U8 *, KMemoryBlock> &allocate) {
        auto first{blocksmap.lower_bound(allocate.first)};
        const auto neededpages{allocate.second.pagescount};
        const auto size{neededpages * SwitchPageSize};

        if (!blocksmap.empty() && first == std::prev(blocksmap.end()))
            --first;
        const auto last{blocksmap.lower_bound(allocate.first + size)};
        if (!allocator->CanAllocate(allocate.first, size))
            throw exception{"Unable to allocate the required block"};

        bool reprotec{allocate.second.state != first->second.state};
        if (!reprotec)
            reprotec = allocate.second.permission != first->second.permission;


        const auto isallocated = [&] (const std::pair<const U8 *, const KMemoryBlock> &blockinfo) {
            const auto blksize{blockinfo.second.pagescount * SwitchPageSize};
            const auto state{blockinfo.second.state};
            return !allocator->CanAllocate(blockinfo.first, blksize)
                && state._type != MemoryTypeValues::Free;
        };

        if (first == last) {
            // first is above the requested allocation address, the previous block is likely free
            if (isallocated(*first))
                --first;
            if (first->second.pagescount < neededpages)
                throw exception{"The previous block is not free"};
            first->second.pagescount -= neededpages;
            blocksmap.insert_or_assign(allocate.first, allocate.second);
            first = blocksmap.lower_bound(allocate.first);

        } else if (first->first + size < last->first && !isallocated(*first)) {
            auto splited{first->second};
            if (allocate.first > first->first) {
                splited.pagescount -= neededpages;
                blocksmap.insert_or_assign(first->first, splited);
                blocksmap.insert_or_assign(allocate.first, allocate.second);
            } else {
                blocksmap.insert_or_assign(first->first, allocate.second);
                splited.pagescount -= neededpages;
                blocksmap.insert_or_assign(allocate.first + size, splited);
            }
        } else {
            // There is an allocated memory space at the end of this block
            first->second.pagescount -= neededpages;
            blocksmap.insert_or_assign(first->first + size, allocate.second);
        }

        if (!isallocated(*first)) {
            if (const auto *available{hostslab->Reserve({}, size)}; available != MemoryFailValue)
                allocator->Map(allocate.first, reinterpret_cast<U64>(available), size);
            else throw exception{"Failed to reserve {} bytes in host memory", size};
        }


        if (const auto perms{allocate.second.permission}; reprotec && perms)
            allocator->Reprotec(allocate.first, size, perms);
    }

    void KMemoryBlockManager::ForEach(const std::pair<U8 *, KMemoryBlock> &blockdesc, std::function<void(KMemoryBlock &)> &&callback) {
        const auto first{FindBlock(blockdesc.first)};
        if (!first)
            throw std::out_of_range{"Block not found"};

        auto *blockit{first.value().second};
        U64 pages{blockdesc.second.pagescount};
        for (; pages; ++blockit) {
            callback(*blockit);
            if (blockit->pagescount > pages)
                break;
            pages -= blockit->pagescount;
        }
    }

    void KMemoryBlockManager::Reprotect(const std::pair<U8 *, KMemoryBlock> &reprotect) {
        const auto block{FindBlock(reprotect.first)};
        if (!block)
            return;
        const auto permission{reprotect.second.permission};
        const auto size{reprotect.second.pagescount * SwitchPageSize};
        if (!size)
            return;
        allocator->Reprotec(reprotect.first, size, permission);
        block->second->permission = permission;
    }

    std::optional<std::pair<const U8 *, KMemoryBlock *>> KMemoryBlockManager::FindBlock(const U8 *guestptr) {
        auto firstblk{blocksmap.lower_bound(guestptr)};
        if (firstblk != blocksmap.end()) {
            if (firstblk->first <= guestptr)
                return std::make_pair(firstblk->first, &firstblk->second);
            if (const auto size{firstblk->second.pagescount * SwitchPageSize})
                if (firstblk->first >= guestptr && firstblk->first + size < guestptr)
                    return std::make_pair(firstblk->first, &firstblk->second);
        }
        if (firstblk->first > guestptr)
            if (--firstblk != blocksmap.end())
                return std::make_pair(firstblk->first, &firstblk->second);
        return std::nullopt;
    }
}
