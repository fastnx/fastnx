#include <ranges>

#include <common/container.h>
#include <common/values.h>
#include <crypto/checksum.h>
#include <fs_sys/vfs/offset_file.h>
#include <fs_sys/nx_fmt/partition_filesystem.h>

namespace FastNx::FsSys::NxFmt {
    PartitionFileSystem::PartitionFileSystem(const VfsBackingFilePtr &pfsf) : VfsReadOnlyDirectory(pfsf->path), partfs(pfsf) {
        if (!IsAPfs0File(partfs))
            return;
        const auto pfs0hd{partfs->Read<Pfs0Header>()};
        ishashfs = pfs0hd.magic == ConstMagicValue<U32>("HFS0");

        constexpr U64 offset{sizeof(pfs0hd)};
        const auto partssize{pfs0hd.fileCount * sizeof(PartitionEntry)};

        pents.reserve(pfs0hd.fileCount);

        U64 nexoffset{offset};
        for (U64 _peindex{}; _peindex < pfs0hd.fileCount; ++_peindex) {
            if (!ishashfs)
                pents.emplace_back(partfs->Read<PartitionEntry>(nexoffset));
            else
                pents.emplace_back(partfs->Read<FileEntryTable>(nexoffset));

            NX_ASSERT(nexoffset - sizeof(pfs0hd) < partssize);
            nexoffset += ishashfs ? sizeof(FileEntryTable) : sizeof(PartitionEntry);
            if (_count >= MaxEntriesCount)
                break;
            _count++;
        }
        files.reserve(_count);
        const auto namestable{partfs->ReadSome<char>(pfs0hd.strTableSize, nexoffset)};
        dataoffset = nexoffset + namestable.size();

        for (const auto &file: pents) {
            // ReSharper disable once CppUseStructuredBinding
            const auto &basefile{get_derived<FileEntryBase>(file)};
            const auto *filetable{namestable.data() + basefile.nameOffset};
            if (!filetable || !basefile.size)
                return;

            std::string filename;
            filename.resize_and_overwrite(strlen(filetable), [&](auto *dest, const U64 size) {
                NX_ASSERT(strlen(filetable) <= pfs0hd.strTableSize);
                std::strncpy(dest, filetable, size);

                return size;
            });

            if (!files.contains(filename))
                files.emplace(std::move(filename), FileEntryBase{dataoffset + basefile.offset, basefile.size, basefile.nameOffset});
        };

        std::ranges::for_each(files, [&](const auto &file) {
            bytesused += file.second.size;
        });
        coverage = CalculateCoverage(bytesused, partfs->GetSize());

        // We no longer need the hashes and raw file entries
        if (ishashfs)
            return;

        auto &&entries{std::move(pents)};
        entries.clear();
        entries.shrink_to_fit();
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

        const auto entry{files.find(_path)};
        if (entry == files.end())
            return nullptr;
        if (pents.empty() && !ishashfs)
            return std::make_shared<Vfs::OffsetFile>(partfs, _path, entry->second.offset, entry->second.size, exists(path));

        Crypto::Checksum hfschecker;
        for (const auto &entryhash: pents) {
            NX_ASSERT(std::holds_alternative<FileEntryTable>(entryhash));
            const auto &hfsfile{std::get<FileEntryTable>(entryhash)};

            // We can compare file metadata through their offset into the filename table
            if (hfsfile.nameOffset != entry->second.nameOffset)
                continue;
            const auto content{partfs->ReadSome(hfsfile.offset, hfsfile.hashregionsize)};

            hfschecker.Update(content);
            const auto result{hfschecker.Finish()};
            if (IsEqual(result, hfsfile.regionhash))
                return std::make_shared<Vfs::OffsetFile>(partfs, _path, dataoffset + hfsfile.offset, hfsfile.size, exists(path));
            break;
        }
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
