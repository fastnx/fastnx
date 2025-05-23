#pragma once
#include <list>

#include <fs_sys/types.h>
namespace FastNx::Core {
    class Assets;
    enum class GamePathType {
        Unrecognized,
        Homebrew,
        ApplicationDirectory,
        Card,
        Shop
    };
    class GamesLists {
    public:
        explicit GamesLists(const std::shared_ptr<Assets> &_assets);
        std::shared_ptr<Assets> assets;

        bool AddTypedGame(const FsSys::FsPath &gamefiles);

        static bool IsFormatAllowed(GamePathType type);
        std::vector<FsSys::FsPath> GetAllGamesPaths(GamePathType type) const;

        std::list<FsSys::FsPath> dirs;
        std::list<std::pair<GamePathType, FsSys::FsPath>> gamespaths;
    };
}
