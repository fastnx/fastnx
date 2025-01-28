#include <common/container.h>
#include <fs_sys/ssd/editable_directory.h>
#include <core/assets.h>
#include <core/games_lists.h>

namespace FastNx::Core {
    GameLists::GameLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        gamesLists.push_back(assets->games);
        for (const auto &defaultDir: ArrayOf<FsSys::FsPath>("~/.local/fastnx/games", "~/.fastnx/games", "~/Documents/games")) {
            gamesLists.emplace_back(defaultDir);
        }
        for (const auto &gameDir: gamesLists) {
            if (!exists(gameDir))
                continue;
            FsSys::Ssd::EditableDirectory directory{gameDir};
            for (const auto gamesRoms{directory.ListAllFiles()}; const auto &game: gamesRoms) {
                gamesLists.emplace_back(game);
            }
        }
    }
}
