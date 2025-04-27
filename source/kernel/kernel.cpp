#include <kernel/kernel.h>
namespace FastNx::Kernel {
    Kernel::Kernel() : devlink(std::make_shared<SysVirtMemory>()), memory(*this) {
        const auto lastpid{pidseed + MaximumProcessIds};
        pidslist.reserve(lastpid - pidseed + 2);
        for (const auto pidval: std::views::iota(pidseed - 1, lastpid + 1)) {
            pidslist.emplace_back(pidval, nullptr);
        }
        NX_ASSERT(std::prev(pidslist.end())->first == lastpid);

        userslabs.emplace(devlink->GetSpan(SlabHeap::BaseAddress, SlabHeap::Size), SwitchPageSize);
    }

}
