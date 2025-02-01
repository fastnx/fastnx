#include <pthread.h>
#include <sched.h>
#include <cpuid.h>
#include <unistd.h>

#include <algorithm>
#include <device/capabilities.h>
template<typename T, FastNx::U64 Size>
auto SmallVector() {
    boost::container::small_vector<T, Size> values(Size);
    return values;
}

boost::container::small_vector<FastNx::U32, 4> FastNx::Device::GetCpuValues() {
    auto values{SmallVector<U32, 4>()};
    assert(values.size() == 4);

    __cpuid(1, values[0], values[1], values[2], values[3]);
    return values;
}


std::pair<std::string, FastNx::I32> FastNx::Device::IsArchSuitable() {
    cpu_set_t cpus;

    I32 activated{};

    if (pthread_getconcurrency())
        return {};

    const auto smt{GetCoresCount()};
    pthread_getaffinity_np(pthread_self(), sizeof(cpus), &cpus);
    activated = 0;
    for (I32 proc{}; proc < smt; proc++) {
        if (CPU_ISSET(proc, &cpus))
            activated++;
    }
    if (activated != smt)
        return {};


    if (const auto vals{GetCpuValues()}; (vals[2] & bit_XSAVE) == 0 || (vals[2] & bit_OSXSAVE) == 0)
        return {};

    auto _xcr0{SmallVector<U32, 4>()};

    // https://www.felixcloutier.com/x86/xgetbv
    __asm__ volatile("XGETBV" : "=a"(_xcr0[0]), "=d"(_xcr0[1]) : "c"(0));

    std::string aspect;
    if ((_xcr0[0] & 6) == 6)
        aspect += "OS Saves YMM, ";
    if (_xcr0[0] & 1 << 7)
        aspect += "CPU Support AVX-512, ";

    const auto rank{std::ranges::count(aspect, ',')};
    if (rank)
        aspect.erase(aspect.find_last_of(','));
    return std::make_pair(aspect, rank);
}
