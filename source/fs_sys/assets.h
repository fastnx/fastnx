#pragma once

#include <fs_sys/games_lists.h>
#include <map>
namespace FastNx::FsSys {
    enum class AssetsType {
        Games
    };
    class Assets : public std::enable_shared_from_this<Assets> {
    public:
        Assets();
        ~Assets();

        void Initialize();

        FsPath directory;
        FsPath games;

        std::optional<GameLists> gLists;
        std::map<AssetsType, FsPath> paths;
    };
}
