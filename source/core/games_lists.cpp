#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <core/assets.h>
#include <loaders/homefs.h>
#include <core/games_lists.h>

namespace FastNx::Core {
    GamesLists::GamesLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        static const auto _games{
            ArrayOf("~/.local/fastnx/games"_fs, "~/.fastnx/games"_fs, "~/Documents/games"_fs)
        };

        directories.push_back(assets->games);
        for (const auto &defaultDir: _games)
            directories.emplace_back(defaultDir);

        for (const auto &gameDir: directories) {
            if (!exists(gameDir))
                continue;
            assert(is_directory(gameDir));
            if (Loaders::IsHomebrewFsDirectory(gameDir)) {
                gamespaths.emplace_back(GamePathType::HomebrewFs, gameDir);
                continue;
            }

            FsSys::ReFs::EditableDirectory directory{gameDir};
            if (!directory)
                continue;
            if (const auto gamesRoms{directory.ListAllFiles()}; !gamesRoms.empty()) {
                for (const auto &game: gamesRoms) {
                    gamespaths.emplace_back(GamePathType::Shop, game);
                }
            }
        }
    }

    std::vector<FsSys::FsPath> GamesLists::GetAllGamesPaths(const GamePathType type) const {
        std::vector<FsSys::FsPath> games(gamespaths.size());
        // ReSharper disable once CppUseStructuredBinding
        for (const auto &gamefile: gamespaths) {
            if (const auto _type{gamefile.first}; _type == type) {
                games.emplace_back(gamefile.second);
            }
        }
        return games;
    }
}
