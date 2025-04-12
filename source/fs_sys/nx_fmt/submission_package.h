#pragma once
#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <horizon/key_set.h>
#include <loaders/gamefs.h>

#include <fs_sys/nx_fmt/content_archive.h>

namespace FastNx::FsSys::NxFmt {
    class SubmissionPackage {
    public:
        explicit SubmissionPackage(const std::shared_ptr<PartitionFileSystem> &pfs, const std::shared_ptr<Horizon::KeySet> &keys);

        U64 titleid{};
        std::shared_ptr<Loaders::GameFileSystem> gfs;
        VfsBackingFilePtr cnmt{nullptr};
        VfsBackingFilePtr corrupted{nullptr};

        std::vector<std::shared_ptr<ContentArchive>> ncalist;
    };
}
