#include <ranges>

#include <fs_sys/nx_fmt/partition_filesystem.h>

namespace FastNx::FsSys::NxFmt {
    PartitionFileSystem::PartitionFileSystem(const VfsBackingFilePtr &pfsf) : VfsReadOnlyDirectory(pfsf->path) {
        if (!IsAPfs0File(pfsf))
            return;
        const auto pfs0hd{pfsf->Read<Pfs0Header>()};

        constexpr U64 _auxOffset{sizeof(pfs0hd)};
        const auto _partsSize{pfs0hd.fileCount * sizeof(PartitionEntry)};

        std::vector<PartitionEntry> pents;
        pents.reserve(pfs0hd.fileCount);

        U64 _nextOffset{_auxOffset};
        for (U64 _pei{}; _pei < pfs0hd.fileCount; ++_pei) {
            pents.emplace_back(pfsf->Read<PartitionEntry>(_nextOffset));

            assert(_nextOffset - sizeof(pfs0hd) < _partsSize);
            _nextOffset += sizeof(PartitionEntry);
            if (_count >= MaxEntriesCount)
                break;
            _count++;
        }
        _files.reserve(_count);
        const auto namestable{pfsf->ReadSome<char>(_nextOffset, pfs0hd.strTableSize)};
        const auto filedata{_nextOffset + namestable.size()};

        for (const auto &file: pents) {
            const auto *fileentry{namestable.data() + file.nameOffset};
            if (!fileentry || !file.size)
                return;

            std::string filename;
            filename.resize_and_overwrite(strlen(fileentry), [&](auto *dest, const U64 size) {
                assert(strlen(fileentry) <= pfs0hd.strTableSize);
                std::strncpy(dest, fileentry, size);

                return size;
            });

            if (!_files.contains(filename))
                _files.emplace(std::move(filename), FileEntryMetadata{filedata + file.offset, file.size});
        };

        auto CountAllBytes = [&](const auto &file) {
            bytesen += file.second.size;
        };
        std::ranges::for_each(_files, CountAllBytes);
    }

    std::vector<FsPath> PartitionFileSystem::ListAllFiles() {
        if (_files.empty())
            return {};
        std::vector<FsPath> result;
        result.reserve(_files.size());
        for (const auto &entryname: std::ranges::views::keys(_files)) {
            result.emplace_back(entryname);
        }
        return result;
    }

    U64 PartitionFileSystem::GetFilesCount() {
        return std::min(_count, _files.size());
    }

    bool IsAValidPfs(const std::shared_ptr<PartitionFileSystem> &spfs) {
        return spfs->GetFilesCount() > 0;
    }
}
