#include <device/capabilities.h>

FastNx::U64 FastNx::Device::GetHostPageSize() {
    static const auto size{sysconf(_SC_PAGESIZE)};
    return static_cast<U64>(size);
}