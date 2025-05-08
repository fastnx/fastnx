#pragma once
#include <flat_map>
#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <horizon/key_set.h>

#include <fs_sys/linkable_directory.h>
#include <fs_sys/nx_fmt/content_archive.h>

namespace FastNx::Loaders {
    class NspEs;
}

namespace FastNx::FsSys::NxFmt {
    class SubmissionPackage {
    public:
        explicit SubmissionPackage(Loaders::NspEs &nsp, const std::shared_ptr<Horizon::KeySet> &_keys);

        void GetAll(Loaders::NspEs &nsp, const VfsBackingFilePtr &cnmt, const std::vector<FsPath> &content);

        U64 titleid{};
        VfsBackingFilePtr cnmt{nullptr};
        VfsBackingFilePtr corrupted{nullptr};
        std::vector<U32> requiredsdk;

        std::shared_ptr<ContentArchive> GetContentNca(const ContentClassifier &type);
    private:
        void ParserContentXml(const VfsBackingFilePtr &metaxml);
        bool Populate(const std::shared_ptr<LinkableDirectory> &directory, std::vector<ContentEnumerate> &meta);
        const std::shared_ptr<PartitionFileSystem> &pfs;
        const std::shared_ptr<Horizon::KeySet> &keys;
        std::map<ContentClassifier, std::shared_ptr<ContentArchive>> ncalist;

        std::flat_map<std::string, U64> contentsize;
    };
}
