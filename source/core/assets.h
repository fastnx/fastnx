#pragma once

#include <map>
#include <core/games_lists.h>
#include <fs_sys/refs/editable_directory.h>
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

        std::optional<FsSys::ReFs::EditableDirectory> directory;
        std::optional<FsSys::ReFs::EditableDirectory> games;
        std::optional<FsSys::ReFs::EditableDirectory> logs;
        std::optional<FsSys::ReFs::EditableDirectory> keys;

        std::optional<GamesLists> gamesLists;
        std::map<AssetsType, FsSys::FsPath> paths;
    };
}
