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
        if (exists(path))
            return;

        openedfd = open64(LandingOf(path), GetIoMode(mode));

        if (openedfd < 0) {
            std::println(std::cerr, "Could not open the file {}", path.string());
            return;
        }
        assert(lockf(openedfd, F_LOCK, 0) == 0);

        struct stat64 status;
        fstat64(openedfd, &status);
        if (auto *buffering{mmap(nullptr, status.st_size, PROT_READ, MAP_PRIVATE, openedfd, 0)}; buffering != MAP_FAILED)
            _aliveVirt = static_cast<U8 *>(buffering);
    }

    HugeFile::~HugeFile() {
        if (_aliveVirt)
            assert(munmap(std::exchange(_aliveVirt, nullptr), _mapSize) == 0);
        assert(lockf(openedfd, F_UNLCK, 0) == 0);
        if (openedfd > 0)
            close(openedfd);
    }

    HugeFile::operator bool() const {
        if (!_aliveVirt || openedfd < 0)
            return {};
        if (lockf(openedfd, F_TEST, 0); errno != EAGAIN)
            return {};

        if (const BufferedFile _osPool{"/proc/self/maps"}; !_osPool)
            return {};

        return true;
    }

#define ENB_FATAL_SIGNAL_ON_ACCESS_TO_RANGE 1

#if ENB_FATAL_SIGNAL_ON_ACCESS_TO_RANGE
#define MADV_GUARD_INSTALL 102
#endif

    U64 HugeFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        auto *_source{&_aliveVirt[offset]};
        if (const auto _alignedOffset{boost::alignment::align_up(offset, 4096)})
            if (msync(_source, _alignedOffset, MS_ASYNC) != 0)
                if (errno == ENOMEM)
                    pagemissRec++;

        if (offset + size > _mapSize)
            return -1;

        std::memcpy(dest, _source, size);

        static auto _flags{MADV_HUGEPAGE | MADV_DONTDUMP};
        [[unlikely]] if (!lastReadFrom) {
#if defined(MADV_GUARD_INSTALL)
            // https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/commit/?id=662df3e5c376
            _flags |= MADV_GUARD_INSTALL;
#endif
            assert(posix_madvise(_aliveVirt, _mapSize, _flags | MADV_SEQUENTIAL) == 0);
        } else if (_source != lastReadFrom) {
            assert(posix_madvise(_aliveVirt, _mapSize, _flags | MADV_RANDOM) == 0);
        }
        if (_source)
            lastReadFrom = _source;
        return size;
    }
}
