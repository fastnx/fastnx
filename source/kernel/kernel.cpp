#include <kernel/kernel.h>
namespace FastNx::Kernel {
    Kernel::Kernel() : nxalloc(std::make_shared<NxAllocator>()), memory(std::make_unique<Memory::KMemory>(*this)) {
        const auto lastpid{pidseed + MaximumProcessIds};
        pidslist.reserve(lastpid - pidseed + 2);
        for (const auto pidval: std::views::iota(pidseed - 1, lastpid + 1)) {
            pidslist.emplace_back(pidval, nullptr);
        }
        NX_ASSERT(std::prev(pidslist.end())->first == lastpid);

        poffset = std::make_shared<Memory::KSlabHeap>(std::span{static_cast<U8 *>(nullptr), SwitchMemorySize}, SwitchPageSize);
    }

}
