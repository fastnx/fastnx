#include <fstream>
#include <fcntl.h>
#include <iostream>

#include <sys/stat.h>
#include <common/container.h>
#include <fs_sys/refs/buffered_file.h>

#include <common/async_logger.h>

namespace FastNx::FsSys::ReFs {
    BufferedFile::BufferedFile(const FsPath &_path, const I32 dirfd, const FileModeType _mode, const bool create) : VfsBackingFile(_path, _mode) {
        if (!exists(path) && create) {
            if (std::fstream file{_path, std::ios::out}; file.is_open())
                file.close();
        }
        if (create)
            NX_ASSERT(mode != FileModeType::ReadOnly);
        descriptor = [&] {
            if (dirfd)
                return openat64(dirfd, GetDataArray(path), ModeToNative(mode));
            return open64(GetDataArray(path), ModeToNative(mode));
        }();

        if (descriptor < 0) {
            AsyncLogger::Error("Could not open the file {}", GetPathStr(path));
            if (create && exists(path))
                std::filesystem::remove(path);
        } else if (mode != FileModeType::ReadOnly) {
            NX_ASSERT(lockf(descriptor, F_LOCK, 0) == 0);
        }
    }

    BufferedFile::~BufferedFile() {
        if (mode != FileModeType::ReadOnly)
            if (lockf(descriptor, F_TLOCK, 0) != 0) {
                if (errno == EACCES || errno == EAGAIN)
                    NX_ASSERT(lockf(descriptor, F_UNLCK, 0) == 0);
        }
        if (descriptor != -1)
            NX_ASSERT(close(descriptor) == 0);
    }

    BufferedFile::operator bool() const {
        I32 attempts{3};
        if (!exists(path) || descriptor == -1)
            attempts--;
        if (mode == FileModeType::None)
            attempts--;

        if (!GetSizeBySeek(descriptor))
            attempts--;
        return attempts > 1;
    }

    U64 BufferedFile::GetSize() const {
        return GetSizeBySeek(descriptor);
    }

    U64 BufferedFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        if (const auto &bufopt{buffer}; !bufopt.empty()) {
            const auto *source{bufopt.data()};
            [[assume(start >= offset)]];
            if (start >= offset && start + valid >= offset + size) {
                if (const auto locally{source + start - offset})
                    std::memcpy(dest, locally, size);
            }
            if (dest == source)
                return size;
        }

        if (const auto _size{std::min(size, 64_KBYTES)}; buffer.size() < _size)
            buffer.resize(_size);
        const auto result = [&] -> U64 {
            U64 copied{};
            for (U64 _offset{offset}; _offset < offset + size;) {
                const auto iosize{buffer.size() < size - copied ? buffer.size() : size - copied};

                U64 retrieved{};
                if (buffer.size() < copied)
                    buffer.resize(copied);
                NX_ASSERT(buffer.size() >= copied);
                if (retrieved = static_cast<U64>(pread64(descriptor, &buffer[copied], iosize, offset)); retrieved == -1ULL) {
                    if (errno == EFAULT)
                        return descriptor = -1;
                    return {};
                }
                start = offset;
                std::memcpy(dest, &buffer[copied], retrieved);
                _offset += retrieved;
                copied += retrieved;
                if (retrieved != iosize)
                    break;
            }
            valid = copied;
            return copied;
        }();
        return result;
    }

    U64 BufferedFile::WriteTypeImpl(const U8 *source, const U64 size, const U64 offset) {
        if (offset >= valid && start + valid < offset + size) {
            valid = start = {};
            std::ranges::fill(buffer, '\0'); // Invalidation of our internal buffer
        }
        if (size > buffer.size()) {
            buffer.resize(size);
        }
        std::memcpy(buffer.data(), source, size);
        const auto copied{pwrite64(descriptor, buffer.data(), size, offset)};

        if (!valid && copied != -1) {
            start = offset;
            valid = copied;
        }
        return copied;
    }
}
