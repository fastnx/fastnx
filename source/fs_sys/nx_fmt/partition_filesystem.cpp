#include <ranges>

#include <common/container.h>
#include <common/values.h>
#include <fs_sys/vfs/offset_file.h>
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
        for (U64 _peindex{}; _peindex < pfs0hd.fileCount; ++_peindex) {
            pents.emplace_back(partfs->Read<PartitionEntry>(_nextOffset));

            NX_ASSERT(_nextOffset - sizeof(pfs0hd) < _partsSize);
            _nextOffset += sizeof(PartitionEntry);
            if (_count >= MaxEntriesCount)
                break;
            _count++;
        }
        files.reserve(_count);
        const auto namestable{partfs->ReadSome<char>(pfs0hd.strTableSize, _nextOffset)};
        const auto filedata{_nextOffset + namestable.size()};

        for (const auto &file: pents) {
            const auto *fileentry{namestable.data() + file.nameOffset};
            if (!fileentry || !file.size)
                return;

            std::string filename;
            filename.resize_and_overwrite(strlen(fileentry), [&](auto *dest, const U64 size) {
                NX_ASSERT(strlen(fileentry) <= pfs0hd.strTableSize);
                std::strncpy(dest, fileentry, size);

                return size;
            });

            if (!files.contains(filename))
                files.emplace(std::move(filename), FileEntryMetadata{filedata + file.offset, file.size});
        };

        std::ranges::for_each(files, [&](const auto &file) {
            bytesused += file.second.size;
        });
        coverage = CalculateCoverage(bytesused, partfs->GetSize());
    }

    std::vector<FsPath> PartitionFileSystem::ListAllFiles() const {
        if (files.empty())
            return {};
        std::vector<FsPath> result;
        result.reserve(files.size());
        for (const auto &entryname: std::ranges::views::keys(files)) {
            result.emplace_back(entryname);
        }
        return result;
    }

    U64 PartitionFileSystem::GetFilesCount() const {
        return std::min(_count, files.size());
    }

    VfsBackingFilePtr PartitionFileSystem::OpenFile(const FsPath &_path, const FileModeType mode) {
        if (_path.has_parent_path())
            return nullptr;
        if (!Contains(ListAllTopLevelFiles(), _path))
            return nullptr;
        NX_ASSERT(mode == FileModeType::ReadOnly);

        if (const auto entry{files.find(_path)}; entry != files.end())
            return std::make_shared<Vfs::OffsetFile>(partfs, _path, entry->second.offset, entry->second.size, exists(path));
        return nullptr;
    }

    bool PartitionFileSystem::Exists(const FsPath &_path) {
        return files.contains(_path);
    }

    bool IsAValidPfs(const std::shared_ptr<PartitionFileSystem> &spfs) {
        return spfs->GetFilesCount() > 0 && spfs->coverage > 95;
    }

    PfsType GetPfsType(const std::shared_ptr<PartitionFileSystem> &pfs) {
        // https://switchbrew.org/wiki/ExeFS

        if (pfs->Exists("main") && pfs->Exists("main.npdm"))
            return PfsType::Exefs;
        if (pfs->Exists("NintendoLogo.png"))
            return PfsType::LogoFs;

        return PfsType::Unknown;
    }
}
