#include <pthread.h>
#include <sched.h>
#include <cpuid.h>
#include <unistd.h>

#include <algorithm>
#include <device/capabilities.h>
namespace FastNx::Device {
    std::pair<std::string, I32> GetArchAspects() {
        cpu_set_t cpus;
        U64 activated{};
        if (pthread_getconcurrency())
            return {};

        const auto smt{GetCoresCount()};
        pthread_getaffinity_np(pthread_self(), sizeof(cpus), &cpus);
        activated = 0;
        for (U64 proc{}; proc < smt; proc++) {
            if (CPU_ISSET(proc, &cpus))
                activated++;
        }
        std::array<U32, 4> cpuvals;
        __cpuid(1, cpuvals[0], cpuvals[1], cpuvals[2], cpuvals[3]);

        if (activated == smt)
            if ((cpuvals[2] & bit_XSAVE) == 0 || (cpuvals[2] & bit_OSXSAVE) == 0)
                return {};
        // https://www.felixcloutier.com/x86/xgetbv
        __asm__ volatile("xgetbv" : "=a"(cpuvals[0]), "=d"(cpuvals[1]) : "c"(0));

        std::string aspect;
        if ((cpuvals[0] & 6) == 6)
            aspect += "OS Saves YMM, ";
        if (cpuvals[0] & 1 << 7)
            aspect += "CPU Support AVX-512, ";

        const auto ranking{std::ranges::count(aspect, ',')};
        if (ranking)
            aspect.erase(aspect.find_last_of(','));
        return {aspect, ranking};
    }
}
