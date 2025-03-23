#include <algorithm>
#include <ranges>
#include <vector>

#include <common/container.h>
#include <kernel/kernel.h>

namespace FastNx::Kernel {
    I32 Kernel::GetPid(const Types::KProcess &process) {
        I32 result{};
        std::scoped_lock lock{idsMutex};
        const auto &values{process.entropy};

        for (auto itPid{std::begin(pidslist)}; itPid != pidslist.end(); ++itPid) {
            if (IsEqual(itPid->second, values))
                return itPid->first;
            if (!result)
                continue;
            NX_ASSERT(std::ranges::contains(itPid->second, 0));

            itPid->second = values;
            result = itPid->first;
        }
        NX_ASSERT(result);
        return result;
    }
}
