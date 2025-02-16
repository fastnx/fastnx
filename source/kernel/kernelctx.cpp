#include <cassert>
#include <mutex>
#include <vector>

#include <kernel/kernelctx.h>
FastNx::I32 FastNx::Kernel::KernelContext::GetPid(const U64 prnde) {
    constexpr auto MaximumProcessId{300};
    static const I32 nxBasePid{processId};
    static std::vector<std::pair<I32, U64>> pids(MaximumProcessId); // Let's search for the PID or create one from scratch

    I32 pid{};
    std::scoped_lock lock(_idsMutex);
    for (I32 _itPid{}; _itPid != pids.size(); _itPid++) {
        if (pids[_itPid].second == prnde) // Search only, immediate return
            return pids[_itPid].first;
        if (pids[_itPid].first == 0)
            if ((pids[_itPid].second = prnde))
                pid = processId = nxBasePid + _itPid;
        assert(pids[_itPid].second > 0);
        if (pid)
            if ((pids[_itPid].second = prnde))
                break;
    }
    assert(pid);
    return pid;
}
