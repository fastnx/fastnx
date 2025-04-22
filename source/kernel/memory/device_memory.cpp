#include <sys/mman.h>
#include <device/memory.h>
#include <kernel/memory/device_memory.h>

namespace FastNx::Kernel::Memory {
    DeviceMemory::DeviceMemory() {
        U64 totalsize{size + 4096};
        if (hostptr = static_cast<U8 *>(Device::AllocateMemory(totalsize, nullptr, -1, false, true)); !hostptr)
            return;

        // Installing a guard page at the end of the memory
        if (mprotect(hostptr + size, 4096, PROT_NONE))
            if (errno == ERANGE) {}
    }
    DeviceMemory::~DeviceMemory() {
        Device::FreeMemory(hostptr, size);
        Device::FreeMemory(hostptr + size, 4096);
    }

    std::span<U8> DeviceMemory::GetSpan(const U64 baseaddr, U64 offset, const bool ishost) const {
        if (!offset)
            offset = size;
        if (!ishost && !guestptr)
            return {};
        return ishost ?
            std::span{hostptr + baseaddr, offset} : std::span{guestptr + baseaddr, offset};
    }

   bool DeviceMemory::InitializeGuestAs(std::span<U8> &addrspace) {
        const auto size{addrspace.size()};
        auto *memory{reinterpret_cast<void *>(size)}; // We must preserve the number of leading zero bits
        auto *result{Device::AllocateGuestMemory(size, memory)};

        if (result == memory)
            if (guestptr = static_cast<U8 *>(result); size)
                if (addrspace = std::span{guestptr, size}; !addrspace.empty())
                    return true;
        return {};
    }
}
