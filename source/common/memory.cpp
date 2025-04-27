#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <common/exception.h>
#include <device/memory.h>
#include <common/memory.h>

namespace FastNx {
    constexpr auto FastNxSharedName{"fastnx_shared"};

    SysVirtMemory::SysVirtMemory() : sharedfd(shm_open(FastNxSharedName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) {
        if (!sharedfd)
            throw exception{"Failed to open the shared object"};

        U64 totalsize{size + 4096};
        if (ftruncate(sharedfd, totalsize))
            throw exception{"We don't have permissions to resize the shared object"};

        if (hostptr = static_cast<U8 *>(Device::AllocateMemory(totalsize, nullptr, sharedfd, false, true)); !hostptr)
            throw exception{"Could not map memory with this FD"};

        // Installing a guard page at the end of the memory
        if (mprotect(hostptr + size, 4096, PROT_NONE))
            if (errno == ERANGE) {}
    }
    SysVirtMemory::~SysVirtMemory() {
        Device::FreeMemory(hostptr, size);
        Device::FreeMemory(hostptr + size, 4096);

        if (sharedfd)
            shm_unlink(FastNxSharedName);
    }

    std::span<U8> SysVirtMemory::GetSpan(const U64 baseaddr, U64 offset, const bool ishost) const {
        if (!offset)
            offset = size;
        if (!ishost && !guestptr)
            return {};
        return ishost ? std::span{hostptr + baseaddr, offset} : std::span{guestptr + baseaddr, offset};
    }

   std::span<U8> SysVirtMemory::InitializeGuestAs(const U64 aswidth) {
        auto *memory{reinterpret_cast<void *>(aswidth)}; // We must preserve the number of leading zero bits
        auto *result{Device::AllocateGuestMemory(size, memory)};

        if (result == memory)
            if (guestptr = static_cast<U8 *>(result); guestptr)
                return std::span{guestptr, size};
        return {};
    }

   void SysVirtMemory::Map(U8 *guest, const U64 hostaddr, U64 size) {
        if (GetSpan(hostaddr, 0, true).empty())
            return;

        if (alloclists.contains(hostaddr))
            return;

        if (mmap(guest, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, sharedfd, hostaddr) != guest)
            throw exception{"Could not map at this address [ guest: {}, host: {:X} ]", fmt::ptr(guest), hostaddr};
        alloclists.emplace(hostaddr, size);
   }
}
