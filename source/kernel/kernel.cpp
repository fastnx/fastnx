#include <kernel/kernel.h>
namespace FastNx::Kernel {
    Kernel::Kernel() : virtmem(std::make_unique<Memory::DeviceLppd4>()) {
        const auto lastpid{pidseed + MaximumProcessIds};
        for (const auto pidval: std::views::iota(pidseed - 1, lastpid + 1)) {
            pidslist.emplace(pidval, std::array<U64, 4>{});
        }
        NX_ASSERT(std::prev(pidslist.end())->first == lastpid);

        void *userobjs{virtmem->device + SlabHeap::BaseAddress};
        userslabs.emplace(userobjs, SlabHeap::Size, SwitchPageSize);
    }

}
