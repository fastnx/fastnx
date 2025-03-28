#include <functional>
#include <fcntl.h>
#include <unistd.h>

#include <common/container.h>
#include <fs_sys/refs/buffered_file.h>
#include <fs_sys/refs/huge_file.h>
#include <fs_sys/refs/editable_directory.h>

namespace FastNx::FsSys::ReFs {

    EditableDirectory::EditableDirectory(const FsPath &_path, const bool create) : VfsBackingDirectory(_path) {
        if (create && !exists(path))
            if (!create_directories(path))
                return;
        if (exists(path))
            NX_ASSERT(is_directory(path));
        if (const auto &dirpath{path}; !dirpath.empty())
            descriptor = open(GetDataArray(dirpath), O_DIRECTORY);
    }

    EditableDirectory::~EditableDirectory() {
        if (descriptor != -1)
            close(descriptor);
    }

    std::vector<FsPath> EditableDirectory::ListAllFiles() const {
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
                    if (const auto opened{openat(descriptor, GetDataArray(walker->path()), O_RDONLY)}; opened > 0) {
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

    std::vector<FsPath> EditableDirectory::ListAllTopLevelFiles() const {
        std::filesystem::directory_iterator walker{path};
        std::vector<FsPath> files;

        while (walker != std::filesystem::directory_iterator{}) {
            files.emplace_back(walker->path());
            ++walker;
        }

        return files;
    }

    U64 EditableDirectory::GetFilesCount() const {
        U64 result{};
        for (const std::filesystem::recursive_directory_iterator walker{path}; const auto &file: walker)
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
                NX_ASSERT(descriptor == -1);
                return {};
            }
        }
        return true;
    }

    VfsBackingFilePtr EditableDirectory::OpenFile(const FsPath &_path, const FileModeType mode) {
        const auto create{mode != FileModeType::ReadOnly};
        const FsPath openpath = [&] {
            if (_path.has_parent_path())
                return _path;
            return path / _path;
        }();

        try {
            if (!Contains(ListAllFiles(), openpath) && !create)
                return nullptr;
        } catch (std::filesystem::filesystem_error &_) {
            bool accessible{};
            for (const auto &subpath: ListAllTopLevelFiles()) {
                if (IsInsideOf(openpath, subpath, false))
                    accessible = true;
            }
            if (!accessible && !create)
                return nullptr;
        }
        if (is_directory(openpath))
            return nullptr;
        if (create && is_regular_file(openpath))
            remove(openpath);

        if (exists(openpath) && file_size(openpath) > 120_MBYTES) {
            return std::make_shared<HugeFile>(openpath, descriptor, mode);
        }
        return std::make_shared<BufferedFile>(openpath, descriptor, mode, create);
    }
}
