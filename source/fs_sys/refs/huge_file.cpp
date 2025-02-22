#include <fstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <boost/align.hpp>

#include <common/container.h>
#include <fs_sys/refs/buffered_file.h>
#include <fs_sys/refs/huge_file.h>
namespace FastNx::FsSys::ReFs {
    HugeFile::HugeFile(const FsPath &_path, const AccessModeType _mode) : VfsBackingFile(_path, _mode) {
        if (!exists(path))
            return;

        openedfd = open64(LandingOf(path), ModeToNative(mode));
        if (openedfd < 0) {
            std::println(std::cerr, "Could not open the file {}", path.string());
            return;
        }
        if (mode != AccessModeType::ReadOnly)
            assert(lockf(openedfd, F_LOCK, 0) == 0);

        struct stat64 status{};
        fstat64(openedfd, &status);
        if (auto *buffering{mmap(nullptr, status.st_size, PROT_READ, MAP_PRIVATE, openedfd, 0)}; buffering != MAP_FAILED)
            memory = static_cast<U8 *>(buffering);
        if (memory)
            mapsize = status.st_size;
    }

    HugeFile::~HugeFile() {
        if (memory)
            assert(munmap(memory, mapsize) == 0);
        if (mode != AccessModeType::ReadOnly)
            assert(lockf(openedfd, F_UNLCK, 0) == 0);
        if (openedfd > 0)
            close(openedfd);
    }

    HugeFile::operator bool() const {
        if (!memory || openedfd < 0)
            return {};
        if (lockf(openedfd, F_TEST, 0); errno != EAGAIN)
            return {};

        if (const BufferedFile _osPool{"/proc/self/maps"}; !_osPool)
            return {};

        return true;
    }

    U64 HugeFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        auto *_source{&memory[offset]};
        const auto aligned{boost::alignment::align_down(_source, 4096)};
        if (const auto _size{boost::alignment::align_up(size, 4096)}; msync(aligned, _size, MS_ASYNC) != 0)
            if (errno == ENOMEM)
                pagemissRec++;

        if (offset + size > mapsize)
            return -1;

        std::memcpy(dest, _source, size);
        if (size >= 4_MEGAS)
            assert(madvise(aligned, size, MADV_REMOVE) == 0);

        static auto _flags{MADV_HUGEPAGE};
        [[unlikely]] if (!recorded) {
            if (madvise(memory, mapsize, _flags | MADV_SEQUENTIAL))
                if (errno == EINVAL)
                    std::terminate();
        } else if (_source < recorded) {
            assert(madvise(memory, mapsize, _flags | MADV_RANDOM) == 0);
        }
        if (_source)
            recorded = _source;
        return size;
    }
}
