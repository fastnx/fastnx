#pragma once

#include <map>
#include <core/games_lists.h>
#include <fs_sys/refs/directory_file_access.h>
namespace FastNx::Core {
    enum class AssetsType {
        Games
    };
    class Assets : public std::enable_shared_from_this<Assets> {
    public:
        Assets();

        void Initialize();
        void Destroy();

        std::vector<FsSys::FsPath> GetAllGames() const;

        std::optional<FsSys::ReFs::DirectoryFileAccess> directory;
        std::optional<FsSys::ReFs::DirectoryFileAccess> games;
        std::optional<FsSys::ReFs::DirectoryFileAccess> logs;
        std::optional<FsSys::ReFs::DirectoryFileAccess> keys;
        std::optional<FsSys::ReFs::DirectoryFileAccess> tiks;

        std::optional<GamesLists> gamesLists;
        std::map<AssetsType, FsSys::FsPath> paths;
    };
}
