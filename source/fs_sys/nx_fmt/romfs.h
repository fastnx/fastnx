#pragma once
#include <map>
#include <fs_sys/types.h>

namespace FastNx::FsSys::NxFmt {


    constexpr auto RomfsInvalidEntry{0xFFFFFFFF};
#pragma pack(push, 1)
    struct RomfsHeader {
        U64 headersize; // Header Length
        U64 dirhashtableoffset;
        U64 dirhashtablesize;
        U64 dirmetaoffset;
        U64 dirmetasize;
        U64 filehashtableoffset;
        U64 filehashtablesize;
        U64 filemetaoffset;
        U64 filemetasize;

        U64 filedataoffset;
    };

    struct RomDirectory {
        U32 parent; // Offset of Parent Directory (self if Root)
        U32 nextdir;
        U32 subdirectory;
        U32 firstfile;
        U32 hashtablenextdir; // Offset of next Directory in the same Hash Table bucket
        U32 namelength;
        char dirname[]; // Name Length (rounded up to multiple of 4)
    };
    struct RomFile {
        U32 diroffset;
        U32 nextfile;
        U64 dataoffset;
        U64 datasize;
        U32 hashtablenextfile;
        U32 namelength;
        char filename[];
    };


#pragma pack(pop)
    class RomFs final : public VfsReadOnlyDirectory {
    public:
        explicit RomFs(const VfsBackingFilePtr &romfs);

        VfsBackingFilePtr OpenFile(const FsPath &_path, FileModeType mode = FileModeType::ReadOnly) override;

        std::vector<FsPath> ListAllFiles() const override;
        [[nodiscard]] std::vector<FsPath> ListAllTopLevelFiles() const override;

    private:
        void TouchFiles(const FsPath &dirname, U64 offset = 0);
        void GetAllFiles(const FsPath &dirname, U64 offset = 0);

        template<typename T>
        auto GetType(const U64 offset) -> const T* {
            if constexpr (std::is_same_v<T, RomFile>) {
                if (offset > filescache.size())
                    return nullptr;
                return reinterpret_cast<const T*>(filescache.data() + offset);
            } else if constexpr (std::is_same_v<T, RomDirectory>) {
                if (offset > dirscache.size())
                    return nullptr;
                return reinterpret_cast<const T*>(dirscache.data() + offset);
            }
            std::unreachable();
        }
        std::vector<U8> filescache, dirscache;
        U64 dataoffset;
        VfsBackingFilePtr backing;

        std::map<FsPath, std::pair<U64, U64>> files;
    };
}
