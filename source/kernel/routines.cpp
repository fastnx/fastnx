#include <algorithm>
#include <common/exception.h>
#include <kernel/kernel.h>
#include <kernel/types/kprocess.h>

namespace FastNx::Kernel {
    U64 Kernel::GetPid(const Types::KProcess &process) {
        std::scoped_lock lock{pmutex};

        for (auto itPid{std::begin(pidslist)}; itPid != pidslist.end(); ++itPid) {
            if (!itPid->second)
                itPid->second = &process.entropy;
            if (itPid->second)
                if (*itPid->second == process.entropy)
                    return itPid->first;
        }
        throw exception{"Could not allocate a PID for the process"};
    }

    std::shared_ptr<Types::KProcess> Kernel::CreateProcess() {
        auto process{std::make_shared<Types::KProcess>(*this)};
        if (process)
            liveprocs.emplace_back(process);
        return process;
    }
}
