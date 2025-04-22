#include <kernel/kernel.h>
namespace FastNx::Kernel {
    Kernel::Kernel() : host(std::make_unique<Memory::DeviceMemory>()), memory(*this) {
        const auto lastpid{pidseed + MaximumProcessIds};
        pidslist.reserve(lastpid - pidseed + 2);
        for (const auto pidval: std::views::iota(pidseed - 1, lastpid + 1)) {
            pidslist.emplace_back(pidval, nullptr);
        }
        NX_ASSERT(std::prev(pidslist.end())->first == lastpid);

        userslabs.emplace(host->GetSpan(SlabHeap::BaseAddress, SlabHeap::Size), SwitchPageSize);
    }

}
