#include <cassert>
#include <mutex>
#include <ranges>
#include <vector>

#include <kernel/kernelctx.h>
FastNx::I32 FastNx::Kernel::KernelContext::GetPid(const U64 prnde) {
    constexpr auto MaximumProcessIds{300};
    static const I32 nxBasePid{processId};
    static std::vector<std::pair<I32, U64>> pids(MaximumProcessIds); // Let's search for the PID or create one from scratch

    I32 pid{};
    std::scoped_lock lock(_idsMutex);
    for (auto _itPid{std::begin(pids)}; _itPid != std::end(pids); ++_itPid) {
        if (_itPid->second == prnde) // Search only, immediate return
            return _itPid->first;
        if (_itPid->first == 0)
            if ((_itPid->second = prnde))
                pid = processId = nxBasePid + std::distance(std::begin(pids), _itPid);
        assert(_itPid->second > 0);
        if (pid)
            if ((_itPid->first = pid))
                break;
    }
    assert(pid);
    return pid;
}
