#include <algorithm>

#include <common/container.h>
#include <common/exception.h>
#include <kernel/kernel.h>

namespace FastNx::Kernel {
    I32 Kernel::GetPid(const Types::KProcess &process) {
        I32 result{};
        std::scoped_lock lock{idsMutex};
        const auto &values{process.entbytes};

        for (auto itPid{std::begin(pidslist)}; itPid != pidslist.end(); ++itPid) {
            if (IsEqual(itPid->second, values))
                return itPid->first;
            if (!result)
                continue;
            itPid->second = values;
            result = itPid->first;
        }
        if (!result)
            throw exception{"Could not allocate a PID for the process"};
        return result;
    }
}
