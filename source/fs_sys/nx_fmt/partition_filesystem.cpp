#include <ranges>

#include <fs_sys/nx_fmt/partition_filesystem.h>
namespace FastNx::FsSys::NxFmt {
    PartitionFileSystem::PartitionFileSystem(const VfsBackingFilePtr &pfsf) : VfsReadOnlyDirectory(pfsf->path) {
    }

    std::vector<FsPath> PartitionFileSystem::ListAllFiles() {
        if (fentries.empty())
            return {};
        std::vector<FsPath> result(fentries.size());
        for (const auto &entryname: std::ranges::views::keys(fentries)) {
            result.emplace_back(entryname);
        }
        return result;
    }

    U64 PartitionFileSystem::GetFilesCount() {
        return std::min(_count, fentries.size());
    }

    bool IsAValidPfs(const std::shared_ptr<PartitionFileSystem> &spfs) {
        return spfs->GetFilesCount() > 0;
    }
}
