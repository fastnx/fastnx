#include <ranges>

#include <common/container.h>
#include <fs_sys/offset_file.h>
#include <fs_sys/nx_fmt/partition_filesystem.h>

namespace FastNx::FsSys::NxFmt {
    PartitionFileSystem::PartitionFileSystem(const VfsBackingFilePtr &pfsf) : VfsReadOnlyDirectory(pfsf->path), partfs(pfsf) {
        if (!IsAPfs0File(partfs))
            return;
        const auto pfs0hd{partfs->Read<Pfs0Header>()};

        constexpr U64 _auxOffset{sizeof(pfs0hd)};
        const auto _partsSize{pfs0hd.fileCount * sizeof(PartitionEntry)};

        std::vector<PartitionEntry> pents;
        pents.reserve(pfs0hd.fileCount);

        U64 _nextOffset{_auxOffset};
        for (U64 _pei{}; _pei < pfs0hd.fileCount; ++_pei) {
            pents.emplace_back(partfs->Read<PartitionEntry>(_nextOffset));

            assert(_nextOffset - sizeof(pfs0hd) < _partsSize);
            _nextOffset += sizeof(PartitionEntry);
            if (_count >= MaxEntriesCount)
                break;
            _count++;
        }
        _files.reserve(_count);
        const auto namestable{partfs->ReadSome<char>(pfs0hd.strTableSize, _nextOffset)};
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

        std::ranges::for_each(_files, [&](const auto &file) {
            bytesused += file.second.size;
        });
        coverage = CalculateCoverage(bytesused, partfs->GetSize());
    }

    std::vector<FsPath> PartitionFileSystem::ListAllFiles() const {
        if (_files.empty())
            return {};
        std::vector<FsPath> result;
        result.reserve(_files.size());
        for (const auto &entryname: std::ranges::views::keys(_files)) {
            result.emplace_back(entryname);
        }
        return result;
    }

    U64 PartitionFileSystem::GetFilesCount() const {
        return std::min(_count, _files.size());
    }

    VfsBackingFilePtr PartitionFileSystem::OpenFile(const FsPath &_path, const FileModeType mode) {
        if (_path.has_parent_path())
            return nullptr;
        if (!Contains(ListAllTopLevelFiles(), _path))
            return nullptr;
        assert(mode == FileModeType::ReadOnly);

        if (const auto entry{_files.find(_path)}; entry != _files.end())
            return std::make_shared<OffsetFile>(partfs, _path, entry->second.offset, entry->second.size, exists(path));
        return nullptr;
    }

    bool IsAValidPfs(const std::shared_ptr<PartitionFileSystem> &spfs) {
        return spfs->GetFilesCount() > 0 && spfs->coverage > 95;
    }
}
