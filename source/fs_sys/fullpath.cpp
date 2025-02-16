#include <regex>
#include <fs_sys/types.h>

namespace FastNx::FsSys {
    std::optional<FsPath> GetFullPath(const FsPath &_path) {
        FsPath path{_path};
        path = std::regex_replace(path.string(), std::regex("~"), secure_getenv("HOME"));
        if (is_symlink(path))
            path = read_symlink(path);

        if (path.is_relative())
            path = relative(path);

        if (exists(path))
            return path;
        return std::nullopt;
    }
}
