#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <common/exception.h>
#include <device/memory.h>
#include <common/memory.h>

namespace FastNx {
    constexpr auto FastNxSharedName{"/fastnx"};

    SysAllocator::SysAllocator(const std::shared_ptr<Jit::PageTable> &table) : MemoryBacking(table), sharedfd(shm_open(FastNxSharedName, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) {
        if (!sharedfd)
            throw exception{"Failed to open the shared object"};

        U64 totalsize{size + PageSize};
        if (ftruncate(sharedfd, totalsize))
            throw exception{"We don't have permissions to resize the shared object"};

        if (hostptr = static_cast<U8 *>(Device::AllocateMemory(totalsize, nullptr, sharedfd, true, true, true)); !hostptr)
            throw exception{"Could not map memory with this FD"};

        // Installing a guard page at the end of the memory
        if (mprotect(hostptr + size, PageSize, PROT_NONE))
            if (errno == ERANGE) {}
    }
    SysAllocator::~SysAllocator() {
        Device::FreeMemory(hostptr, size);
        Device::FreeMemory(hostptr + size, PageSize);

        if (sharedfd)
            shm_unlink(FastNxSharedName);
    }

    std::span<U8> SysAllocator::GetSpan(const U64 baseaddr, U64 offset, const bool ishost) const {
        if (!offset)
            offset = size;
        if (!ishost && !guestptr)
            return {};
        return ishost ? std::span{hostptr + baseaddr, offset} : std::span{guestptr + baseaddr, offset};
    }

   std::span<U8> SysAllocator::InitializeGuestAs(const U64 aswidth, const U64 assize) {
        constexpr auto AsanAvailableMap{0x600000000000};
        auto *memory{reinterpret_cast<void *>(aswidth + AsanAvailableMap)}; // We must preserve the number of leading zero bits
        auto *result{Device::AllocateGuestMemory(assize, memory)};

        if (result == memory)
            if (guestptr = static_cast<U8 *>(result); guestptr)
                return std::span{guestptr, assize};
        if (guestptr = static_cast<U8 *>(Device::AllocateGuestMemory(assize)); guestptr)
            return {guestptr, assize};
        return {};
    }

   void SysAllocator::Map(U8 *guest, const U64 hostaddr, U64 size) {
        if (GetSpan(hostaddr, 0, true).empty())
            return;

        if (alloclists.contains({guest, hostaddr}))
            return;

        if (mmap(guest, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, sharedfd, hostaddr) != guest)
            throw exception{"Could not map at this address [ guest: {}, host: {:#x} ], errno: {}", fmt::ptr(guest), hostaddr, errno};

        pagetable->CreateTable(guest, hostptr, hostaddr, size);
        alloclists.emplace(std::make_pair(guest, hostaddr), size);

#if !defined(NDEBUG)
        std::memset(guest, 0, size);
#endif
   }

   bool SysAllocator::CanAllocate(const U8 *region, const U64 size) {
        for (const auto &[allocinfo, _size]: alloclists) {
            const auto [guestmemory, _] = allocinfo;
            if (guestmemory <= region && guestmemory + _size > region)
                if (_size && size)
                    return {};
        }
        return true;
   }

   void SysAllocator::Reprotec(U8 *guest, const U64 size, const I32 protection) {
        const auto prots = [&] {
            I32 result{};
            if (protection & 1)
                result |= PROT_READ;
            if (protection & 1 << 1)
                result |= PROT_WRITE;

            // Useless due to JIT
            /*
            if (protection & 1 << 2)
                result |= PROT_EXEC;
            */
            return result;
        }();

        if (mprotect(guest, size, prots))
            if (errno == ERANGE)
                throw exception{"Could not re-protect at this address"};
   }
}
