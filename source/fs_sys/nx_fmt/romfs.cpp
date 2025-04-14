
#include <common/exception.h>
#include <fs_sys/vfs/offset_file.h>
#include <fs_sys/nx_fmt/romfs.h>



namespace FastNx::FsSys::NxFmt {

    auto Valid(const U32 value) {
        return value != RomfsInvalidEntry;
    }
    template<typename T>
    FsPath GetName(const T &value) {
        const auto length{value.namelength};
        if (!length || !Valid(length))
            return {};
        if constexpr (std::is_same_v<T, RomDirectory>) {
            return std::string_view{value.dirname, length};
        } else if (std::is_same_v<T, RomFile>) {
            return std::string_view{value.filename, length};
        }
        std::unreachable();
    }


    void RomFs::TouchFiles(const FsPath &dirname, const U64 offset) {
        if (const auto *file{GetType<RomFile>(offset)}) {
            if (const auto &path{dirname / GetName(*file)}; !path.empty()) {
                if (!files.contains(path))
                    files.emplace(std::move(path), std::make_pair(file->dataoffset, file->datasize));
            }
            if (Valid(file->nextfile))
                TouchFiles(dirname, file->nextfile);
        }
    }
    void RomFs::GetAllFiles(const FsPath &dirname, const U64 offset) {
        const auto *directory{GetType<RomDirectory>(offset)};

        if (Valid(directory->subdirectory))
            GetAllFiles(dirname / GetName(*directory), directory->subdirectory);
        if (Valid(directory->firstfile))
            TouchFiles(dirname / GetName(*directory), directory->firstfile);

        if (Valid(directory->nextdir))
            GetAllFiles(dirname / GetName(*directory), directory->nextdir);
    }

    RomFs::RomFs(const VfsBackingFilePtr &romfs): VfsReadOnlyDirectory(romfs->path), backing(romfs) {
        if (romfs->GetSize() < sizeof(RomfsHeader))
            return;

        const auto rootfs{romfs->Read<RomfsHeader>()};
        if (rootfs.headersize != sizeof(RomfsHeader))
            throw exception{"Unsupported RomFs version"};

        dataoffset = rootfs.filedataoffset;
        filescache = romfs->ReadSome(rootfs.filemetasize, rootfs.filemetaoffset);
        dirscache = romfs->ReadSome(rootfs.dirmetasize, rootfs.dirmetaoffset);

        GetAllFiles("/");

    }

    VfsBackingFilePtr RomFs::OpenFile(const FsPath &_path, const FileModeType mode) {
        NX_ASSERT(mode == FileModeType::ReadOnly);
        if (const auto fileit{files.find(_path)}; fileit != files.end()) {
            if (const auto [offset, size]{fileit->second}; size)
                return std::make_shared<Vfs::OffsetFile>(backing, fileit->first, dataoffset + offset, size);
        }
        return nullptr;
    }

    std::vector<FsPath> RomFs::ListAllFiles() const {
        std::vector<FsPath> result;
        for (const auto &file: files | std::ranges::views::keys) {
            result.emplace_back(file);
        }
        return result;
    }

    std::vector<FsPath> RomFs::ListAllTopLevelFiles() const {
        std::unreachable();
    }
}
