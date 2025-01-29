#pragma once

#include <fs_sys/nx_fmt/partition_filesystem.h>
#include <loaders/types.h>
namespace FastNx::Loaders {
    class NspEs : public AppLoader {
    public:
        explicit NspEs(const FsSys::VfsBackingFilePtr &nspf, bool& isLoaded);

    private:
        std::shared_ptr<FsSys::NxFmt::PartitionFileSystem> _mainPfs;
    };
}
