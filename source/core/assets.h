#pragma once

#include <core/games_lists.h>
#include <map>
namespace FastNx::Core {
    enum class AssetsType {
        Games
    };
    class Assets : public std::enable_shared_from_this<Assets> {
    public:
        Assets();

        void Initialize();
        void Destroy();

        FsSys::FsPath directory;
        FsSys::FsPath games;

        std::optional<GamesLists> gamesLists;
        std::map<AssetsType, FsSys::FsPath> paths;
    };
}
