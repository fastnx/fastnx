#pragma once
#include <list>

#include <fs_sys/types.h>
namespace FastNx::FsSys {
    class Assets;

    class GameLists {
    public:
        explicit GameLists(const std::shared_ptr<Assets>& _assets);

        std::shared_ptr<Assets> assets;
        std::list<FsPath> gamesLists;

        std::list<FsPath> gamespaths;
    };
}
