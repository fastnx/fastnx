#include <thread>
#include <device/capabilities.h>

#include <fs_sys/refs/editable_directory.h>
#include <fs_sys/refs/buffered_file.h>

FastNx::U64 FastNx::Device::GetCoresCount() {
    auto count{static_cast<U64>(sysconf(_SC_NPROCESSORS_ONLN))};
    const auto threads{std::thread::hardware_concurrency()};

    if (FsSys::ReFs::BufferedFile smt{"/sys/devices/system/cpu/smt/active"}; smt && count != threads) {
        if (smt.Read<char>() == '1')
            count *= 2;
    }
    assert(count == threads);

    if (FsSys::ReFs::EditableDirectory cores{"/sys/devices/system/cpu"}) {
        const auto cpus{cores.BlobAllFiles("cpu*")};
        I32 threadable{};
        for (const auto& cpuef : cpus)
            if (std::isdigit(cpuef.string().back()))
                threadable++;
        count = threadable;
    }

    cpu_set_t cpus;
    pthread_getaffinity_np(pthread_self(), sizeof(cpus), &cpus);

    if (CPU_COUNT(&cpus) > count)
        count = CPU_COUNT(&cpus);
    return count;
}
