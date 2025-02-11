#pragma once
#include <list>

#include <fs_sys/types.h>
namespace FastNx::Core {
    class Assets;
    enum class GamePathType {
        Homebrew,
        HomebrewFs,
        Card,
        Shop
    };
    class GamesLists {
    public:
        explicit GamesLists(const std::shared_ptr<Assets> &_assets);
        std::shared_ptr<Assets> assets;

        std::vector<FsSys::FsPath> GetAllGamesPaths(GamePathType type) const;

        std::list<FsSys::FsPath> directories;
        std::list<std::pair<GamePathType, FsSys::FsPath>> gamespaths;
    };
}
