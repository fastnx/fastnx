#include <sys/mman.h>
#include <fmt/format.h>

#include <boost/align.hpp>

#include <fs_sys/refs/buffered_file.h>

#include <device/capabilities.h>
#include <device/memory.h>
namespace FastNx::Device {
    void *AllocateMemory(U64 &size, void *fixed, const I32 file, bool enbhuge, const bool writable, const bool shared) {
        auto prots{PROT_READ};
        auto flags{shared ? MAP_SHARED : MAP_PRIVATE};

        enbhuge = file == -1 ? enbhuge : false;
        if (writable)
            prots |= PROT_WRITE;
        if (file == -1)
            flags |= MAP_ANONYMOUS;

        if (enbhuge)
            flags |= MAP_HUGETLB;
        if (fixed && !file) {
            flags |= MAP_FIXED;
            if (GetMemorySize(fixed) == size)
                return fixed;
        } else if (file != -1) {
            fixed = {};
            size = FsSys::GetSizeBySeek(file);
        }

        const auto result{mmap(fixed, size, prots, flags, file, 0)};
        size = boost::alignment::align_up(size, GetHostPageSize());
        if (result == MAP_FAILED)
            throw std::bad_alloc{};
        return result;
    }

    void *AllocateGuestMemory(const U64 &size, void *fixed) {
        constexpr auto prots{PROT_READ | PROT_WRITE};
        constexpr auto flags{MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED};

        const auto result{mmap(fixed, size, prots, flags, -1, 0)};
        if (result == MAP_FAILED)
            throw std::bad_alloc{};
        NX_ASSERT(madvise(result, size, MADV_DONTNEED) == 0);
        return result;
    }

    void FreeMemory(void *allocated, U64 size) {
        size = boost::alignment::align_up(size, GetHostPageSize());

        if (const auto rsize{GetMemorySize(allocated)}; rsize != size) {
#if 0
            NX_ASSERT(madvise(allocated, size, MADV_REMOVE) == 0);
#else
            NX_ASSERT(munmap(allocated, size) == 0);
#endif
        } else if (munmap(allocated, size))
            switch (errno) {
                case EINVAL:
                    throw std::bad_alloc{};
                default:
                    break;
            }
    }

    U64 GetMemorySize(const void *allocated) {
        const auto strrep{fmt::format("{:x}", reinterpret_cast<U64>(allocated))};
        if (FsSys::ReFs::BufferedFile maps{"/proc/self/maps"}) {
            for (const auto& line : maps.GetAllLines()) {
                if (!line.starts_with(strrep))
                    continue;

                const auto region{line | std::views::drop(line.find_first_of('-') + 1)};
                const auto endregion{std::strtoul(region.begin().base(), nullptr, 0x10)};
                return endregion - reinterpret_cast<U64>(allocated);
            }
        }
        return {};
    }
}
