#include <functional>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>

#include <base/container.h>
#include <fs_sys/ssd/editable_directory.h>
namespace FastNx::FsSys::SSD {
    EditableDirectory::EditableDirectory(const FsPath &_path, const bool create) : VfsBackingDirectory(_path) {
        if (create && !exists(path))
            if (!create_directories(path))
                return;
        assert(is_directory(path));
        if (const auto &dirpath{path}; !dirpath.empty())
            descriptor = open(LandingOf(dirpath), O_DIRECTORY);
    }
    std::vector<FsPath> EditableDirectory::ListAllFiles() {
        std::vector<FsPath> paths;
        std::filesystem::directory_iterator walker{path};

        std::function<void(std::filesystem::directory_iterator&)> FindAllFiles = [&](std::filesystem::directory_iterator& entry) {
            for (; entry != std::filesystem::directory_iterator{}; ++entry) {
                if (entry->is_regular_file()) {
                    paths.emplace_back(entry->path());
                    if (const auto opened{openat(descriptor, entry->path().c_str(), O_RDONLY)}; opened > 0) {
                        close(opened);
                    }
                }
                if (entry->is_directory())
                    FindAllFiles(entry);
            }
        };
        FindAllFiles(walker);
        return paths;
    }
}
