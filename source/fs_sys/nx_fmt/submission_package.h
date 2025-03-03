#pragma once
#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <loaders/romdir_mkfs.h>

namespace FastNx::FsSys::NxFmt {
    class SubmissionPackage {
    public:
        explicit SubmissionPackage(const std::shared_ptr<PartitionFileSystem> &pfs);

        std::shared_ptr<Loaders::RomDirMkFs> _rfs;
        VfsBackingFilePtr cnmt{nullptr};
        VfsBackingFilePtr corrupted{nullptr};
    };
}
