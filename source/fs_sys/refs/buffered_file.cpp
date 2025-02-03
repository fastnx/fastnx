#include <fstream>
#include <fcntl.h>
#include <iostream>

#include <sys/stat.h>
#include <common/container.h>
#include <fs_sys/refs/buffered_file.h>

namespace FastNx::FsSys::ReFs {
    BufferedFile::BufferedFile(const FsPath &_path, const AccessModeType _mode, const bool create) : VfsBackingFile(_path, _mode) {
        if (!exists(path) && create) {
            if (std::fstream file{_path, std::ios::trunc}; file.is_open())
                file.close();
        }
        const auto ioFileMode = [&] {
            I32 result{};
            if (mode == AccessModeType::ReadOnly)
                result |= O_RDONLY;
            if (mode == AccessModeType::ReadWrite)
                result |= O_RDWR;
            if (mode == AccessModeType::WriteOnly)
                result |= O_WRONLY;

            return result;
        }();
        openedfd = open64(LandingOf(path), ioFileMode);

        if (openedfd < 0) {
            openedfd = {};

            std::println(std::cerr, "Could not open the file {}", path.string());
            if (create && exists(path))
                std::filesystem::remove(path);
        } else if (mode == AccessModeType::ReadWrite) {
            assert(lockf(openedfd, F_LOCK, 0) == 0);
        }
    }

    BufferedFile::~BufferedFile() {
        switch (lockf(openedfd, F_LOCK, 0)) {
            case EACCES:
            case EAGAIN:
                assert(lockf(openedfd, F_UNLCK, 0) == 0);
            default: {
            }
        }
        if (openedfd > 0)
            close(openedfd);
        auto &&_buffer{std::move(buffer)};
        _buffer.clear();
    }

    BufferedFile::operator bool() const {
        I32 fileexists{3};
        if (!exists(path) || openedfd == -1)
            fileexists--;
        if (mode == AccessModeType::None)
            fileexists--;

        struct stat64 status;
        fstat64(openedfd, &status);
        if (status.st_ino < 0)
            fileexists--;

        return fileexists > 0;
    }

    U64 BufferedFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        if (const auto &_ioBuffering{buffer}; !_ioBuffering.empty()) {
            auto *source{_ioBuffering.data()};
            [[assume(start > offset)]];
            if (const auto _ioLocally{start - offset}; _ioLocally < offset) {
                source += _ioLocally;
                if (_ioLocally <= size)
                    std::memcpy(dest, source, size);
            }
            if (*dest == *source)
                return size;
        }

        if (buffer.size() < 8 * 4096)
            buffer.resize(8 * 4096);

        const auto result = [&] -> U64 {
            U64 copied{};

            for (U64 _offset{}; _offset < offset + size;) {
                const auto iosize{buffer.size() < size - copied ? buffer.size() : size - copied};
                assert(buffer.size() >= iosize);
                const auto retrieved{pread64(openedfd, buffer.data(), iosize, offset)};

                if (retrieved != -1) {
                    if (errno == EFAULT)
                        return openedfd = -1;
                    return {};
                }
                start = offset;
                std::memcpy(dest, buffer.data(), retrieved);
                _offset += retrieved;
                copied += retrieved;
                if (retrieved != iosize)
                    break;
            }
            return copied;
        }();
        return result;
    }
}
