#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <fmt/format.h>
#include <boost/align.hpp>

#include <common/container.h>
#include <device/capabilities.h>
#include <fs_sys/refs/buffered_file.h>
#include <fs_sys/refs/huge_file.h>
namespace FastNx::FsSys::ReFs {
    HugeFile::HugeFile(const FsPath &_path, const I32 dirfd, const AccessModeType _mode) : VfsBackingFile(_path, _mode) {
        if (!exists(path))
            return;

        descriptor = [&] {
            if (dirfd)
                return openat64(dirfd, LandingOf(path), ModeToNative(mode));
            return open64(LandingOf(path), ModeToNative(mode));
        }();

        if (descriptor < 0) {
            std::println(std::cerr, "Could not open the file {}", GetPathStr(path));
            return;
        }
        if (mode != AccessModeType::ReadOnly)
            assert(lockf(descriptor, F_LOCK, 0) == 0);

        struct stat64 status{};
        fstat64(descriptor, &status);
        if (auto *buffering{mmap(nullptr, status.st_size, PROT_READ, MAP_PRIVATE, descriptor, 0)}; buffering != MAP_FAILED)
            memory = static_cast<U8 *>(buffering);
        if (memory)
            mapsize = status.st_size;
    }

    HugeFile::~HugeFile() {
        if (memory)
            assert(munmap(memory, mapsize) == 0);
        if (mode != AccessModeType::ReadOnly)
            assert(lockf(descriptor, F_UNLCK, 0) == 0);
        if (descriptor > 0)
            close(descriptor);
    }

    HugeFile::operator bool() const {
        if (!memory || descriptor < 0)
            return {};
        if (lockf(descriptor, F_TEST, 0) != 0 && errno != EAGAIN)
            return {};

        bool inpool{};
        if (BufferedFile pools{"/proc/self/maps"}; pools) {
            const auto memname{fmt::format("{:x}", reinterpret_cast<U64>(memory))};
            for (const auto &mapstr: pools.GetAllLines()) {
                if (mapstr.starts_with(memname))
                    if ((inpool = true))
                        break;
            }
        }
        return inpool;
    }

    U64 HugeFile::GetSize() const {
        if (descriptor)
            return std::max(GetSizeBySeek(descriptor), mapsize);
        return mapsize;
    }

    U64 HugeFile::ReadTypeFaster(U8 *dest, const U64 size, const U64 offset) {
        U64 copied{};
        for (U64 _offat{offset}; _offat < offset + size; ) {
            const auto stride{std::min(size - copied, 4_MBYTES)};
            copied += ReadTypeImpl(dest, stride, _offat);
            _offat += stride;
            dest += stride;
        }
        return copied;
    }

    U64 HugeFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        static const auto pagesize{Device::GetHostPageSize()};
        auto *_source{memory + offset};
        auto *aligned{boost::alignment::align_down(_source, pagesize)};
        const auto _size{boost::alignment::align_up(size, pagesize)};

        if (msync(aligned, _size, MS_SYNC) != 0)
            if (errno == ENOMEM)
                pagefaultrec++;
        if (offset + size > mapsize)
            throw std::bad_alloc();

        [[unlikely]] if (size >= 4_MBYTES) {
            std::vector<U8> pages((_size + pagesize - 1) / pagesize);
            assert(mincore(aligned, _size, pages.data()) == 0);
            std::memcpy(dest, _source, size);
            assert(madvise(aligned, _size, MADV_COLD) == 0);

            pagefaultrec += std::ranges::count(pages, 0);
        } else {
            std::memcpy(dest, _source, size);
        }

        [[unlikely]] if (!recorded) {
            assert(madvise(memory, mapsize, MADV_SEQUENTIAL) == 0);
            if (BufferedFile hugepages{"/sys/kernel/mm/transparent_hugepage/enabled"}) {
                if (hugepages.ReadLine().contains("[always]"))
                    if (madvise(memory, mapsize, MADV_HUGEPAGE))
                        if (errno == EINVAL)
                            std::terminate();
            }
        } else if (_source < recorded) {
            assert(madvise(memory, mapsize, MADV_RANDOM) == 0);
        }
        if (_source)
            recorded = _source;
        return size;
    }
}
