#include <functional>
#include <cassert>
#include <fcntl.h>
#include <unistd.h>

#include <common/container.h>
#include <fs_sys/ssd/editable_directory.h>

namespace FastNx::FsSys::Ssd {

    EditableDirectory::EditableDirectory(const FsPath &_path, const bool create) : VfsBackingDirectory(_path) {
        if (create && !exists(path))
            if (!create_directories(path))
                return;
        if (exists(path))
            assert(is_directory(path));
        if (const auto &dirpath{path}; !dirpath.empty())
            descriptor = open(LandingOf(dirpath), O_DIRECTORY);
    }

    std::vector<FsPath> EditableDirectory::ListAllFiles() {
        std::vector<FsPath> filepaths;
        for (const auto& folders : ArrayOf<FsPath>("/etc")) {
            if (path == folders)
                filepaths.reserve(1024);
        }
        if (!filepaths.capacity())
            filepaths.reserve(GetFilesCount());

        std::function<void(const std::filesystem::directory_entry &)> FindAllFiles = [&](const std::filesystem::directory_entry &entry) {
            if (const EditableDirectory directory{entry}; !directory)
                return;

            for (std::filesystem::directory_iterator walker{entry}; walker != std::filesystem::directory_iterator{}; ++
                 walker) {
                if (walker->is_regular_file()) {
                    filepaths.emplace_back(walker->path());
                    if (const auto opened{openat(descriptor, LandingOf(walker->path()), O_RDONLY)}; opened > 0) {
                        close(opened);
                    } else {
                        filepaths.pop_back();
                    }
                } else if (walker->is_directory()) {
                    FindAllFiles(*walker);
                }
            }
        };
        FindAllFiles(std::filesystem::directory_entry{path});
        return filepaths;
    }

    U64 EditableDirectory::GetFilesCount() {
        U64 result{};
        for (const std::filesystem::recursive_directory_iterator walker{path}; const auto& file : walker)
            if (file.is_regular_file())
                result++;
        return result;
    }

    EditableDirectory::operator bool() const {
        if (!exists(path))
            return {};
        const auto _directory{status(path)};
        if (_directory.type() != std::filesystem::file_type::directory)
            return {};

        using std::filesystem::perms;
        if (const auto _perms{_directory.permissions()}; _perms != perms{}) {
            if ((_perms & perms::others_exec) != perms::others_exec) {
                assert(descriptor == -1);
                return {};
            }
        }
        return true;
    }
}
