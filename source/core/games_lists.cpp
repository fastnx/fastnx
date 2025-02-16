#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <core/assets.h>
#include <loaders/homefs.h>
#include <core/games_lists.h>

namespace FastNx::Core {
    GamesLists::GamesLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        const auto _games{
            ArrayOf("~/.local/fastnx/Games"_fs, "~/.fastnx/Games"_fs, "~/Documents/Games"_fs)
        };

        for (const auto &hardpath: _games) {
            if (const auto _realPath{FsSys::GetFullPath(hardpath)})
                _dirs.emplace_back(*_realPath);
        }
        if (exists(assets->games))
            _dirs.emplace_back(assets->games);

        for (const auto &gameDir: _dirs) {
            FsSys::ReFs::EditableDirectory directory{gameDir};
            if (!directory)
                continue;

            assert(is_directory(gameDir));
            if (!AddTypedGame(gameDir)) {
                if (const auto &_roms{directory.ListAllFiles()}; !_roms.empty()) {
                    for (const auto &_gameFiles: _roms)
                        assert(AddTypedGame(_gameFiles) == true);
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
