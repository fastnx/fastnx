#include <thread>
#include <numeric>

#define ENB_CPU_SET_USAGE 0

#if ENB_CPU_SET_USAGE == 0
#include <algorithm>
#include <common/container.h>
#endif

#include <device/capabilities.h>
#include <fs_sys/refs/directory_file_access.h>
#include <fs_sys/refs/buffered_file.h>

FastNx::U64 FastNx::Device::GetCoresCount() {
    auto count{static_cast<U64>(sysconf(_SC_NPROCESSORS_ONLN))};
    const auto threads{std::thread::hardware_concurrency()};

    if (FsSys::ReFs::BufferedFile smt{"/sys/devices/system/cpu/smt/active"}; smt && count != threads) {
        if (smt.Read<char>() == '1')
            count *= 2;
    }
    NX_ASSERT(count == threads);

    if (const FsSys::ReFs::DirectoryFileAccess cores{"/sys/devices/system/cpu"}) {
        const auto cpus{cores.GlobAllFiles("cpu*")};
        I32 threadable{};
        for (const auto &cpuef: cpus)
            if (std::isdigit(cpuef.string().back()))
                threadable++;
        count = threadable;
    }

#if ENB_CPU_SET_USAGE
    cpu_set_t cpus;
    pthread_getaffinity_np(pthread_self(), sizeof(cpus), &cpus);

    if (CPU_COUNT(&cpus) > count)
        count = CPU_COUNT(&cpus);
#else
    std::vector<U64> _maskCount(32 / sizeof(U64)); // Possible with the AMD EPYC Genoa 9004 Series Processor Family
    NX_ASSERT(SizeofVector(_maskCount) == 32);
    NX_ASSERT(sched_getaffinity(getpid(), SizeofVector(_maskCount), reinterpret_cast<cpu_set_t *>(_maskCount.data())) == 0);
    if (I32 result{}; std::ranges::fold_left(_maskCount, 0, std::plus())) {
        for (const auto &_mask: _maskCount) {
            if (_mask)
                result += std::popcount(_mask);
        }
        count = result;
    }

#endif
    return count;
}
