#include <common/container.h>
#include <fs_sys/ssd/editable_directory.h>
#include <core/assets.h>
#include <core/games_lists.h>

namespace FastNx::Core {
    GamesLists::GamesLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        gamesLists.push_back(assets->games);
        for (const auto &defaultDir: ArrayOf("~/.local/fastnx/games", "~/.fastnx/games", "~/Documents/games")) {
            gamesLists.emplace_back(defaultDir);
        }
        for (const auto &gameDir: gamesLists) {
            FsSys::Ssd::EditableDirectory directory{gameDir};
            if (!directory)
                continue;
            const auto gamesRoms{directory.ListAllFiles()};
            if (gamesRoms.empty())
                continue;

            for (const auto &game: gamesRoms) {
                gamesLists.emplace_back(game);
            }
        }
    }
}
