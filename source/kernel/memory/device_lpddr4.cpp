#include <sys/mman.h>
#include <device/memory.h>
#include <kernel/memory/device_lpddr4.h>

namespace FastNx::Kernel::Memory {
    DeviceLppd4::DeviceLppd4() {
        U64 totalsize{size + 4096};
        if (const auto memory{static_cast<U8 *>(Device::AllocateMemory(totalsize, nullptr, -1, false, true))})
            hostptr = memory;

        // Installing a guard page at the end of the memory
        if (mprotect(hostptr + size, 4096, PROT_NONE))
            if (errno == ERANGE) {}
    }
    DeviceLppd4::~DeviceLppd4() {
        Device::FreeMemory(hostptr, size);
        Device::FreeMemory(hostptr + size, 4096);
    }

    MemoryDescriptor DeviceLppd4::GetDescriptor(const U64 baseAddr, const U64 size) const {
        auto *begin{hostptr + baseAddr};
        return MemoryDescriptor{
            .begin = begin,
            .end = begin + size
        };
    }
}
