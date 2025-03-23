#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <fmt/format.h>
#include <boost/align.hpp>

#include <common/container.h>
#include <common/exception.h>
#include <device/capabilities.h>
#include <common/async_logger.h>
#include <device/memory.h>

#include <fs_sys/refs/buffered_file.h>
#include <fs_sys/refs/huge_file.h>

namespace FastNx::FsSys::ReFs {
    HugeFile::HugeFile(const FsPath &_path, const I32 dirfd, const FileModeType _mode) : VfsBackingFile(_path, _mode) {
        if (!exists(path))
            return;

        descriptor = [&] {
            if (dirfd)
                return openat64(dirfd, GetDataArray(path), ModeToNative(mode));
            return open64(GetDataArray(path), ModeToNative(mode));
        }();

        if (descriptor < 0) {
            AsyncLogger::Error("Could not open the file {}", GetPathStr(path));
            return;
        }
        const auto writable{mode != FileModeType::ReadOnly};
        if (writable)
            NX_ASSERT(lockf(descriptor, F_LOCK, 0) == 0);

        memory = static_cast<U8 *>(Device::AllocateMemory(mapsize, nullptr, descriptor, true, writable));
    }

    HugeFile::~HugeFile() {
        if (memory)
            Device::FreeMemory(memory, mapsize);
        if (mode != FileModeType::ReadOnly)
            NX_ASSERT(lockf(descriptor, F_UNLCK, 0) == 0);
        if (descriptor > 0)
            NX_ASSERT(close(descriptor) == 0);
    }

    HugeFile::operator bool() const {
        if (memory || descriptor > 0)
            if (lockf(descriptor, F_TEST, 0) == 0 && errno != EAGAIN)
                return Device::GetMemorySize(memory) == mapsize;
        return {};
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
        if (!memory)
            return {};
        auto *source{memory + offset};
        auto *aligned{boost::alignment::align_down(source, pagesize)};
        const auto _size{boost::alignment::align_up(size, pagesize)};

        if (msync(aligned, _size, MS_SYNC) != 0)
            if (errno == ENOMEM)
                pagefaultrec++;
        if (offset + size > mapsize)
            throw std::bad_alloc{};

        [[unlikely]] if (size >= 4_MBYTES) {
            std::vector<U8> pages((_size + pagesize - 1) / pagesize);
            NX_ASSERT(mincore(aligned, _size, pages.data()) == 0);
            std::memcpy(dest, source, size);
            NX_ASSERT(madvise(aligned, _size, MADV_COLD) == 0);

            pagefaultrec += std::ranges::count(pages, 0);
        } else {
            std::memcpy(dest, source, size);
        }

        [[unlikely]] if (!recorded) {
            NX_ASSERT(madvise(memory, mapsize, MADV_SEQUENTIAL) == 0);
            if (BufferedFile hugepages{"/sys/kernel/mm/transparent_hugepage/enabled"}) {
                if (hugepages.ReadLine().contains("[always]"))
                    if (madvise(memory, mapsize, MADV_HUGEPAGE))
                        if (errno == EINVAL)
                            throw exception{"It is not possible to access the specified range"};
            }
        } else if (source < recorded) {
            NX_ASSERT(madvise(memory, mapsize, MADV_RANDOM) == 0);
        }
        if (source)
            recorded = source;
        return size;
    }

    U64 HugeFile::WriteTypeImpl(const U8 *source, const U64 size, const U64 offset) {
        const auto _size{offset + size};
        if (_size > mapsize)
            if (ftruncate(descriptor, _size) == 0)
                mapsize = _size;

        if (mapsize < _size)
            throw std::bad_alloc{};
        std::memcpy(&memory[offset], source, size);

        return size;
    }
}
