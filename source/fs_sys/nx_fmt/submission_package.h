#pragma once
#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <horizon/key_set.h>
#include <loaders/application_directory.h>

#include <fs_sys/linkable_directory.h>
#include <fs_sys/nx_fmt/content_archive.h>

namespace FastNx::FsSys::NxFmt {
    class SubmissionPackage {
    public:
        explicit SubmissionPackage(const std::shared_ptr<PartitionFileSystem> &pfs, const std::shared_ptr<Horizon::KeySet> &_keys);

        void GetAll(const VfsBackingFilePtr &cnmt, const std::vector<FsPath> &content);

        U64 titleid{};
        std::shared_ptr<Loaders::ApplicationDirectory> appdir;
        VfsBackingFilePtr cnmt{nullptr};
        VfsBackingFilePtr corrupted{nullptr};

        std::shared_ptr<ContentArchive> GetContentNca(const ContentClassifier &type);
    private:
        bool Populate(const std::shared_ptr<LinkableDirectory> &directory, std::vector<ContentEnumerate> &meta);
        const std::shared_ptr<PartitionFileSystem> &files;
        const std::shared_ptr<Horizon::KeySet> &keys;
        std::map<ContentClassifier, std::shared_ptr<ContentArchive>> ncalist;
    };
}
