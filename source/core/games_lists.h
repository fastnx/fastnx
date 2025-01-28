#pragma once
#include <list>

#include <fs_sys/types.h>
namespace FastNx::Core {
    class Assets;
    class GameLists {
    public:
        explicit GameLists(const std::shared_ptr<Assets> &_assets);
        std::shared_ptr<Assets> assets;

        std::list<FsSys::FsPath> gamesLists;
        std::list<FsSys::FsPath> gamespaths;
    };
}
