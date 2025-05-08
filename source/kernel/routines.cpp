#include <algorithm>
#include <common/exception.h>
#include <kernel/threads/kscheduler.h>
#include <kernel/kernel.h>
#include <kernel/types/kprocess.h>


namespace FastNx::Kernel {
    U64 Kernel::GetPid(const ProcessEntropy &processent) {
        std::scoped_lock lock{pmutex};

        for (auto itPid{std::begin(pidslist)}; itPid != pidslist.end(); ++itPid) {
            if (!itPid->second)
                itPid->second = &processent;
            if (itPid->second)
                if (*itPid->second == processent)
                    return itPid->first;
        }
        throw exception{"Could not allocate a PID for the process"};
    }

    std::shared_ptr<Types::KThread> Kernel::CreateThread() {
        const auto thread{std::make_shared<Types::KThread>(*this)};
        if (thread)
            scheduler->Emplace(thread);
        return thread;
    }

    std::shared_ptr<Types::KProcess> Kernel::CreateProcess() {
        const auto process{std::make_shared<Types::KProcess>(*this)};
        if (process)
            liveprocs.emplace_back(process);
        return process;
    }
}
