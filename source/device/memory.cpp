#include <sys/mman.h>
#include <fmt/format.h>

#include <boost/align.hpp>

#include <fs_sys/refs/buffered_file.h>

#include <device/capabilities.h>
#include <device/memory.h>
namespace FastNx::Device {
    void *AllocateMemory(U64 &size, void *fixed, const I32 file, bool enbhuge, const bool writable) {
        auto prots{PROT_READ};
        auto flags{MAP_PRIVATE};

        enbhuge = !file ? enbhuge : false;
        if (writable)
            prots |= PROT_WRITE;
        if (!file)
            flags |= MAP_ANONYMOUS;

        if (enbhuge)
            flags |= MAP_HUGETLB;
        if (fixed && !file) {
            flags |= MAP_FIXED;
            if (GetMemorySize(fixed) == size)
                return fixed;
        } else if (file) {
            fixed = {};
            size = FsSys::GetSizeBySeek(file);
        }

        const auto result{mmap(fixed, size, prots, flags, file, 0)};
        size = boost::alignment::align_up(size, GetHostPageSize());
        if (result == MAP_FAILED)
            throw std::bad_alloc{};
        return result;
    }

    void FreeMemory(void *allocated, U64 size) {
        size = boost::alignment::align_up(size, GetHostPageSize());

        if (const auto rsize{GetMemorySize(allocated)}; rsize != size) {
            assert(madvise(allocated, size, MADV_REMOVE) == 0);
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
