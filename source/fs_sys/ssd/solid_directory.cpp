#include <cassert>
#include <fcntl.h>

#include <base/container.h>
#include <fs_sys/ssd/solid_directory.h>
namespace FastNx::FsSys::SSD {
    SolidDirectory::SolidDirectory(const FsPath &_path, const bool create) : path(_path) { // NOLINT(*-pass-by-value)
        if (create && !exists(path))
            if (!create_directories(path))
                return;
        assert(is_directory(path));
        if (const auto &dirpath{path}; !dirpath.empty())
            descriptor = open(GetStr(dirpath), O_DIRECTORY);
    }
}
