#pragma once

#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <fs_sys/nx_fmt/submission_package.h>
#include <horizon/key_set.h>
#include <loaders/types.h>

namespace FastNx::Loaders {
    class NspEs : public AppLoader {
    public:
        explicit NspEs(const FsSys::VfsBackingFilePtr &nspf, const std::shared_ptr<Horizon::KeySet> &keys, bool &isLoaded);

    private:
        std::shared_ptr<FsSys::NxFmt::PartitionFileSystem> _mainPfs;
        std::shared_ptr<FsSys::NxFmt::SubmissionPackage> subnsp;
    };
}
