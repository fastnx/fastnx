#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <map>
#include <mutex>

#include <common/container.h>
#include <fs_sys/types.h>

namespace FastNx::FsSys {
    U64 GetFileFsn(const I32 fd) {
        struct stat64 fds;
        fstat64(fd, &fds);
        return fds.st_ino;
    }

    U64 GetSizeBySeek(const I32 fd) {
        static std::map<U64, U64> sizes;
        static std::mutex mutex;

        std::lock_guard lock(mutex);

        const auto fsn{GetFileFsn(fd)};
        if (fsn && sizes.contains(fsn)) // The `strace` output was being flooded with seeks
            return sizes[fsn];

        const auto _current{lseek(fd, 0, SEEK_CUR)};
        const auto result{lseek64(fd, 0, SEEK_END)};
        if (fsn)
            sizes[fsn] = result;

        lseek64(fd, _current, SEEK_SET);
        return result;
    }

    I32 ModeToNative(const AccessModeType type) {
        if (type == AccessModeType::ReadOnly)
            return O_RDONLY;
        if (type == AccessModeType::ReadWrite)
            return O_RDWR;
        if (type == AccessModeType::WriteOnly)
            return O_WRONLY;

        std::unreachable();
    }

    std::string VfsBackingFile::ReadLine(const U64 offset) {
        std::vector<char> content;
        while (!Contains(content, '\n')) {
            const auto linesize{content.size() + 32};
            content.resize(linesize);

            const auto size{ReadSome(content, offset)};
            if (size != linesize)
                content.resize(size);
            if (size < linesize)
                break;
        }
        std::string result(content.begin(), content.end());
        if (const auto position{result.find_first_of('\n')}; !result.empty())
            if (position != std::string::npos)
                result.erase(position);
        return result;
    }

    std::vector<std::string> VfsBackingFile::GetAllLines() {
        std::vector<std::string> result;
        U64 offset{};
        do {
            const auto line{ReadLine(offset)};
            if (line.empty())
                break;
            offset += line.size() + 1;
            result.emplace_back(std::move(line));
        } while (offset);
        return result;
    }
}
