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
        treemap.insert_or_assign(addrspace.begin().base(), KMemoryBlock{
            .pagescount = count,
            .state = MemoryState{MemoryTypeValues::Free}
        });
        // Fix to avoid breaking the std::map algorithm
        treemap.insert_or_assign(addrspace.begin().base() + count * SwitchPageSize, KMemoryBlock{
            .pagescount = {},
            .state = MemoryState{MemoryTypeValues::Inaccessible}
        });

        // ReSharper disable once CppDFALocalValueEscapesFunction
        return addrspace;
    }

    void KMemoryBlockManager::Map(const std::pair<U8 *, KMemoryBlock> &allocate) {
        auto first{treemap.lower_bound(allocate.first)};
        const auto neededpages{allocate.second.pagescount};
        const auto size{neededpages * SwitchPageSize};

        if (first == std::prev(treemap.end()))
            --first;
        const auto last{treemap.lower_bound(allocate.first + size)};
        if (!allocator->CanAllocate(allocate.first, size))
            throw exception{"Address already allocated"};

        bool reprotec{allocate.second.state != first->second.state};
        if (!reprotec)
            reprotec = allocate.second.permission != first->second.permission;


        const bool isallocated{first->second.state != MemoryState{MemoryTypeValues::Free}};

        if (first == last) {
            first->second.pagescount -= neededpages;

            treemap.insert_or_assign(first->first + size, first->second);
            treemap.insert_or_assign(allocate.first, allocate.second);

        } else if (first->first + size < last->first && !isallocated) {
            auto splited{first->second};
            if (allocate.first > first->first) {
                splited.pagescount -= neededpages;
                treemap.insert_or_assign(first->first, splited);
                treemap.insert_or_assign(allocate.first, allocate.second);
            } else {
                treemap.insert_or_assign(first->first, allocate.second);
                splited.pagescount -= neededpages;
                treemap.insert_or_assign(allocate.first + size, splited);
            }
        } else {
            // There is an allocated memory space at the end of this block
            first->second.pagescount -= neededpages;
            treemap.insert_or_assign(first->first + size, allocate.second);
        }

        if (!isallocated) {
            if (const auto *hostoffset{hostslab->Reserve(nullptr, size)}; hostoffset != MemoryFailValue)
                allocator->Map(allocate.first, reinterpret_cast<U64>(hostoffset), size);
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
        auto firstblk{treemap.lower_bound(guestptr)};
        if (firstblk != treemap.end()) {
            if (firstblk->first <= guestptr)
                return std::make_pair(firstblk->first, &firstblk->second);
            if (const auto size{firstblk->second.pagescount * SwitchPageSize})
                if (firstblk->first >= guestptr && firstblk->first + size < guestptr)
                    return std::make_pair(firstblk->first, &firstblk->second);
        }
        if (firstblk->first > guestptr)
            if (--firstblk != treemap.end())
                return std::make_pair(firstblk->first, &firstblk->second);
        return std::nullopt;
    }
}
