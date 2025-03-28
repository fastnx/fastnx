#include <device/memory.h>
#include <kernel/dram/device_lpddr4.h>

namespace FastNx::Kernel::Dram {
    DeviceLppd4::DeviceLppd4() : devram(static_cast<U8 *>(Device::AllocateMemory(size, nullptr, -1, false, true))) {
    }

    DeviceLppd4::~DeviceLppd4() {
        Device::FreeMemory(devram, size);
    }
}
