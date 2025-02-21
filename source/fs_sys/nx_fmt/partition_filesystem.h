#pragma once

#include <map>

#include <common/types.h>
#include <common/traits.h>
#include <fs_sys/types.h>
namespace FastNx::FsSys::NxFmt {
    constexpr auto MaxEntriesCount{0x10};

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
    static_assert(TraitSizeMatch<Pfs0Header, 16>);
    static_assert(TraitSizeMatch<PartitionEntry, 0x14 + 0x4>);

    struct FileEntryMetadata {
        U64 offset, size;
    };
    class PartitionFileSystem final : public VfsReadOnlyDirectory {
    public:
        explicit PartitionFileSystem(const VfsBackingFilePtr &pfsf);

        std::vector<FsPath> ListAllFiles() override;
        U64 GetFilesCount() override;

        std::vector<FsPath> ListAllTopLevelFiles() const override {
            return {};
        }
    private:
        std::map<std::string, FileEntryMetadata> fentries;
        U64 _count{};
    };

    bool IsAValidPfs(const std::shared_ptr<PartitionFileSystem>& spfs);
}
