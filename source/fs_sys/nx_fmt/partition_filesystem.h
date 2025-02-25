#pragma once

#include <boost/container/flat_map.hpp>

#include <common/types.h>
#include <common/traits.h>
#include <fs_sys/types.h>
namespace FastNx::FsSys::NxFmt {

    // https://switchbrew.org/wiki/NCA
    struct Pfs0Header {
        U32 magic;
        U32 fileCount;
        U32 strTableSize;
        [[deprecated("This field is usually zeroed")]] U32 zeroed;
    };
    struct PartitionEntry {
        U64 offset, size;
        U32 nameOffset;
        [[deprecated]] U32 version;
    };
    static_assert(IsSizeMatch<Pfs0Header, 16>);
    static_assert(IsSizeMatch<PartitionEntry, 0x14 + 0x4>);

    struct FileEntryMetadata {
        U64 offset, size;
    };
    class PartitionFileSystem final : public VfsReadOnlyDirectory {
    public:
        static constexpr auto MaxEntriesCount{0x10};

        explicit PartitionFileSystem(const VfsBackingFilePtr &pfsf);

        std::vector<FsPath> ListAllFiles() override;
        U64 GetFilesCount() override;

        [[nodiscard]] std::vector<FsPath> ListAllTopLevelFiles() const override {
            return {};
        }
        U8 coverage{};
    private:
        boost::container::flat_map<std::string, FileEntryMetadata> _files;
        U64 _count{};
        U64 bytesused{}; // Byte counter used by the data of all files in this partition
    };

    bool IsAValidPfs(const std::shared_ptr<PartitionFileSystem> &spfs);
}
