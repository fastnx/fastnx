#pragma once

#include <map>
#include <fs_sys/types.h>

namespace FastNx::FsSys {
    class LinkableDirectory final : public VfsReadOnlyDirectory {
    public:
        explicit LinkableDirectory(const FsPath &path) : VfsReadOnlyDirectory(path) {}
        bool LinkFile(const FsPath &_path, const VfsBackingFilePtr &file);

        bool DirectoryLinkFiles(const std::string &dirname, const VfsReadOnlyDirectoryPtr &directory);

        VfsBackingFilePtr OpenFile(const FsPath &_path, FileModeType mode = FileModeType::ReadOnly) override;
        std::vector<FsPath> ListAllFiles() const override;
        [[nodiscard]] std::vector<FsPath> ListAllTopLevelFiles() const override;


        U64 GetFilesCount() const override;
        bool Exists(const FsPath &_path) override;
    private:
        std::map<FsPath, VfsBackingFilePtr> files;
    };
}
