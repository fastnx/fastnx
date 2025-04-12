#pragma once

#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <fs_sys/nx_fmt/submission_package.h>
#include <horizon/key_set.h>
#include <loaders/types.h>

namespace FastNx::Loaders {
    class NspEs final : public AppLoader {
    public:
        explicit NspEs(const FsSys::VfsBackingFilePtr &nspf, const std::shared_ptr<Horizon::KeySet> &keys, bool &isloaded);

        std::vector<U8> GetLogo() override;
        U64 GetTitleId() override;
    private:
        std::shared_ptr<FsSys::NxFmt::PartitionFileSystem> _mainpfs;
        std::shared_ptr<FsSys::NxFmt::SubmissionPackage> subnsp;
    };
}
