#pragma once

#include <map>
#include <core/games_lists.h>
#include <fs_sys/refs/directory_file_access.h>
namespace FastNx::Core {
    enum class AssetsType {
        Games
    };
    enum class AssetFileType {
        Setupfile
    };

    class Assets : public std::enable_shared_from_this<Assets> {
    public:
        Assets();

        void Initialize();
        void Destroy();
        void LoadGamesLists();

        std::vector<FsSys::FsPath> GetAllGames() const;

        std::optional<FsSys::ReFs::DirectoryFileAccess> directory;
        std::optional<FsSys::ReFs::DirectoryFileAccess> games;
        std::optional<FsSys::ReFs::DirectoryFileAccess> logs;
        std::optional<FsSys::ReFs::DirectoryFileAccess> keys;
        std::optional<FsSys::ReFs::DirectoryFileAccess> tiks;

        std::optional<GamesLists> gameslists;
        std::map<AssetsType, FsSys::FsPath> paths;


        FsSys::VfsBackingFilePtr GetFile(AssetFileType type);
    };
}
