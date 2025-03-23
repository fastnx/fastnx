

#include <kernel/kernel.h>
namespace FastNx::Kernel {
    Kernel::Kernel() {
        const auto lastpid{pidseed + MaximumProcessIds};
        for (const auto pidval: std::views::iota(pidseed - 1, lastpid + 1)) {
            pidslist.emplace(pidval, std::array<U64, 4>{});
        }
        NX_ASSERT(std::prev(pidslist.end())->first == lastpid);
    }

}
