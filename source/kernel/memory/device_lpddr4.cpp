#include <device/memory.h>
#include <kernel/memory/device_lpddr4.h>

namespace FastNx::Kernel::Memory {
    DeviceLppd4::DeviceLppd4() : device(static_cast<U8 *>(Device::AllocateMemory(size, nullptr, -1, false, true))) {
    }

    DeviceLppd4::~DeviceLppd4() {
        Device::FreeMemory(device, size);
    }
}
