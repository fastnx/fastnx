#include <device/capabilities.h>

#include <fs_sys/ssd/editable_directory.h>
#include <fs_sys/ssd/buffered_file.h>

FastNx::U64 FastNx::Device::GetCoresCount() {
    auto count{static_cast<U64>(sysconf(_SC_NPROCESSORS_CONF))};
    if (FsSys::Ssd::BufferedFile smt{"/sys/devices/system/cpu/smt/active"}) {
        if (smt.Read<U32>() == 1)
            count *= 2;
    }

    if (FsSys::Ssd::EditableDirectory cores{"/sys/devices/system/cpu"})
        count = cores.BlobAllFiles("cpu*").size();

    cpu_set_t cpus;
    pthread_getaffinity_np(pthread_self(), sizeof(cpus), &cpus);

    if (CPU_COUNT(&cpus) > count)
        count = CPU_COUNT(&cpus);
    return count;
}
