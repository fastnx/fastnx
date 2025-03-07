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

        const auto curoff{lseek(fd, 0, SEEK_CUR)};
        const auto result{lseek64(fd, 0, SEEK_END)};
        if (fsn)
            sizes[fsn] = result;

        lseek64(fd, curoff, SEEK_SET);
        return result;
    }

    I32 ModeToNative(const FileModeType type) {
        if (type == FileModeType::ReadOnly)
            return O_RDONLY;
        if (type == FileModeType::ReadWrite)
            return O_RDWR;
        if (type == FileModeType::WriteOnly)
            return O_WRONLY;

        std::unreachable();
    }

    std::string VfsBackingFile::ReadLine(const U64 offset) {
        constexpr auto BufferSize{32};
        std::string content;
        content.resize(BufferSize);

        for (U64 linesize{}; !Contains(content, '\n'); ) {
            if (content.size() < linesize)
                break;
            linesize += BufferSize;

            content.resize(linesize);
            if (const auto size{ReadType(content.begin().base(), content.size(), offset)}; size != linesize)
                content.resize(size);
        }
        if (const auto &newline{std::ranges::find(content, '\n')}; newline != content.end())
            content.erase(newline, content.end());
        return content;
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
