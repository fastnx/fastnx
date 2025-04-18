#include <algorithm>

#include <common/container.h>
#include <common/exception.h>
#include <kernel/kernel.h>
#include <kernel/types/kprocess.h>

namespace FastNx::Kernel {
    U64 Kernel::GetPid(const Types::KProcess &process) {
        U64 pident{};
        std::scoped_lock lock{idsMutex};
        for (auto itPid{std::begin(pidslist)}; itPid != pidslist.end() && !pident; ++itPid) {
            if (IsEqual(itPid->second, process.entropy))
                return itPid->first;
            if (!IsZeroes(itPid->second))
                continue;
            itPid->second = process.entropy;
            pident = itPid->first;
        }
        if (!pident)
            throw exception{"Could not allocate a PID for the process"};
        return pident;
    }

    std::shared_ptr<Types::KProcess> Kernel::CreateProcess() {
        auto process{std::make_shared<Types::KProcess>(*this)};
        if (process)
            liveprocs.emplace_back(process);
        return process;
    }
}
