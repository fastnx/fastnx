#pragma once

#include <fs_sys/cnmt.h>

namespace FastNx::Loaders {
    class ApplicationDirectory {
    public:
        ApplicationDirectory(const FsSys::VfsReadOnlyDirectoryPtr &content, std::vector<FsSys::ContentEnumerate> &&metadata);
        explicit ApplicationDirectory(const FsSys::VfsReadOnlyDirectoryPtr &files);

        void ExtractAllFiles() const;

        FsSys::VfsBackingFilePtr GetNpdm() const;
        FsSys::VfsReadOnlyDirectoryPtr GetExefs() const;

        const FsSys::VfsReadOnlyDirectoryPtr appdir;
        std::vector<FsSys::ContentEnumerate> contentenum;
    };

    bool IsApplicationDirectory(const FsSys::FsPath &dirfs);
}
