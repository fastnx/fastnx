#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <loaders/homefs.h>
#include <core/assets.h>
#include <core/games_lists.h>

namespace FastNx::Core {
    GamesLists::GamesLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        const auto gamesfiles{
            ArrayOf("~/.local/fastnx/Games", "~/.fastnx/Games", "~/Documents/Games")
        };

        for (const auto &hardpath: gamesfiles) {
            if (const auto realPath{FsSys::GetFullPath(hardpath)})
                dirs.emplace_back(*realPath);
        }
        if (exists(assets->games->path))
            dirs.emplace_back(assets->games->path);

        for (const auto &gameDir: dirs) {
            FsSys::ReFs::EditableDirectory directory{gameDir};
            if (!directory)
                continue;

            NX_ASSERT(is_directory(gameDir));
            if (!AddTypedGame(gameDir)) {
                if (const auto &_roms{directory.ListAllFiles()}; !_roms.empty()) {
                    for ([[maybe_unused]] const auto &gamefiles: _roms)
                        NX_ASSERT(AddTypedGame(gamefiles) == true);
                }
            }
        }
    }

    bool GamesLists::AddTypedGame(const FsSys::FsPath &gamefiles) {
        GamePathType _type{};
        if (is_directory(gamefiles)) {
            if (Loaders::IsHomebrewFsDirectory(gamefiles))
                _type = GamePathType::HomebrewFs;
        } else {
            if (gamefiles.extension() == ".xci")
                _type = GamePathType::Card;
            else
                _type = GamePathType::Shop;
        }
        if (_type != GamePathType::Unrecognized) {
            gamespaths.emplace_back(_type, gamefiles);
            return true;
        }

        return {};
    }

    std::vector<FsSys::FsPath> GamesLists::GetAllGamesPaths(const GamePathType type) const {
        std::vector<FsSys::FsPath> games{};
        // ReSharper disable once CppUseStructuredBinding
        for (const auto &gamefile: gamespaths) {
            if (const auto _type{gamefile.first}; _type == type) {
                games.emplace_back(gamefile.second);
            }
        }
        return games;
    }
}
