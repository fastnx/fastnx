#include <functional>
#include <fcntl.h>
#include <mutex>
#include <unistd.h>

#include <common/container.h>
#include <fs_sys/refs/buffered_file.h>
#include <fs_sys/refs/huge_file.h>
#include <fs_sys/refs/directory_file_access.h>

namespace FastNx::FsSys::ReFs {

    DirectoryFileAccess::DirectoryFileAccess(const FsPath &_path, const bool create) : VfsBackingDirectory(_path) {
        if (create && !exists(path))
            if (!create_directories(path))
                return;
        if (exists(path))
            NX_ASSERT(is_directory(path));
        if (const auto &dirpath{path}; !dirpath.empty())
            descriptor = open(GetDataArray(dirpath), O_DIRECTORY);
    }

    DirectoryFileAccess::~DirectoryFileAccess() {
        if (descriptor != -1)
            close(descriptor);
    }

    std::vector<FsPath> DirectoryFileAccess::ListAllFiles() const {
        std::vector<FsPath> filepaths;
        for (const auto &folders: ArrayOf<FsPath>("/etc")) {
            if (path == folders)
                filepaths.reserve(1024);
        }
        if (!filepaths.capacity())
            filepaths.reserve(GetFilesCount());
        using FsDir = std::filesystem::directory_entry;
        std::function<void(const FsDir &)> ForeachAllFiles = [&](const FsDir &entry) {
            if (const DirectoryFileAccess directory{entry}; !directory)
                return;

            for (std::filesystem::directory_iterator treewalk{entry}; treewalk != decltype(treewalk){}; ++treewalk) {
                if (treewalk->is_regular_file() || treewalk->is_symlink()) {
                    filepaths.emplace_back(treewalk->path());
                    if (const auto result{faccessat(descriptor, GetDataArray(treewalk->path()), R_OK, AT_EACCESS)}; result == -1)
                        filepaths.pop_back();
                } else if (treewalk->is_directory()) {
                    ForeachAllFiles(*treewalk);
                }
            }
        };
        ForeachAllFiles(FsDir{path});
        return filepaths;
    }

    std::vector<FsPath> DirectoryFileAccess::ListAllTopLevelFiles() const {
        std::filesystem::directory_iterator treewalk{path};
        std::vector<FsPath> files;

        while (treewalk != std::filesystem::directory_iterator{}) {
            files.emplace_back(treewalk->path());
            ++treewalk;
        }
        return files;
    }
    U64 DirectoryFileAccess::GetFilesCount() const {
        U64 result{};
        for (const std::filesystem::recursive_directory_iterator walker{path}; const auto &file: walker)
            if (file.is_regular_file())
                result++;
        return result;
    }

    DirectoryFileAccess::operator bool() const {
        if (!exists(path))
            return {};
        const auto _directory{status(path)};
        if (_directory.type() != std::filesystem::file_type::directory)
            return {};

        using std::filesystem::perms;
        if (const auto subperms{_directory.permissions()}; subperms != perms{}) {
            if ((subperms & perms::others_exec) != perms::others_exec) {
                NX_ASSERT(descriptor == -1);
                return {};
            }
        }
        return true;
    }

    VfsBackingFilePtr DirectoryFileAccess::OpenFile(const FsPath &_path, const FileModeType mode) {
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
#if 0
        if (create && is_regular_file(openpath))
            remove(openpath);
#endif
        std::lock_guard lock{spinlock};
        if (const auto file{filelist.find(openpath)}; file != filelist.end()) {
            if (file->second->mode == mode)
                return file->second;
            filelist.erase(file);
        }

        const auto file = [&] -> VfsBackingFilePtr {
            if (exists(openpath) && file_size(openpath) > 120_MBYTES) {
                return std::make_shared<HugeFile>(openpath, descriptor, mode);
            }
            return std::make_shared<BufferedFile>(openpath, descriptor, mode, create);
        }();
        filelist[openpath] = file;
        return file;
    }

    bool DirectoryFileAccess::Exists(const FsPath &_path) {
        if (_path.root_path() == _path)
            return exists(_path);
        return false;
    }
}
