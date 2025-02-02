#include <fstream>
#include <fcntl.h>

#include <common/container.h>
#include <fs_sys/ssd/buffered_file.h>
namespace FastNx::FsSys::Ssd {
    BufferedFile::BufferedFile(const FsPath &_path, const AccessModeType _mode, const bool create) : VfsBackingFile(_path, _mode) {
        if (create) {
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
        openedfd = open64(LandingOf(_path), ioFileMode);
    }

    BufferedFile::operator bool() const {
        if (openedfd == -1)
            return {};
        return true;
    }

    U64 BufferedFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        if (const auto& _ioBuffering{buffer}; !_ioBuffering.empty()) {
            auto* source{_ioBuffering.data()};
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

            for (U64 _offset{}; _offset < offset + size; ) {
                const auto iosize{buffer.size() < size - copied ? buffer.size() : size - copied};
                assert(buffer.size() >= iosize);
                const auto retrieved{pread64(openedfd, buffer.data(), iosize, offset)};

                if (retrieved != -1) {
                    start = offset;
                    std::memcpy(dest, buffer.data(), retrieved);
                    _offset += retrieved;
                    copied += retrieved;
                }
                if (retrieved != iosize)
                    break;

                assert(fdatasync(openedfd) == 0);
            }
            return copied;
        }();
        return result;
    }
}
