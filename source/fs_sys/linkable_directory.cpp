#include <fs_sys/linkable_directory.h>

namespace FastNx::FsSys {
    bool LinkableDirectory::LinkFile(const FsPath &_path, const VfsBackingFilePtr &file) {
        if (files.contains(_path))
            return false;
        files.emplace(_path, file);
        return true;
    }

    bool LinkableDirectory::DirectoryLinkFiles(const std::string &dirname, const VfsReadOnlyDirectoryPtr &directory) {
        for (const auto &files: directory->ListAllFiles()) {
            if (!LinkFile(dirname / files, directory->OpenFile(files)))
                return {};
        }

        return true;
    }

    VfsBackingFilePtr LinkableDirectory::OpenFile(const FsPath &_path, const FileModeType mode) {
        if (mode == FileModeType::ReadOnly)
            if (files.contains(_path))
                return files[_path];
        return nullptr;
    }

    std::vector<FsPath> LinkableDirectory::ListAllFiles() const {
        std::vector<FsPath> result;
        result.reserve(files.size());
        for (const auto &filename: files | std::views::keys)
            result.emplace_back(filename);
        return result;
    }
    std::vector<FsPath> LinkableDirectory::ListAllTopLevelFiles() const {
        return {};
    }

    U64 LinkableDirectory::GetFilesCount() const {
        return files.size();
    }
}
