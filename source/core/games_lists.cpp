#include <common/async_logger.h>
#include <common/container.h>
#include <common/exception.h>

#include <fs_sys/refs/directory_file_access.h>
#include <loaders/application_directory.h>

#include <core/application.h>
#include <core/assets.h>
#include <core/games_lists.h>


namespace FastNx::Core {
    GamesLists::GamesLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        const auto gamesfiles{
            ArrayOf("~/.local/fastnx/Games", "~/.fastnx/Games", "~/Documents/Games")
        };

        for (const auto &hardpath: gamesfiles) {
            if (const auto realpath{FsSys::GetFullPath(hardpath)})
                dirs.emplace_back(*realpath);
        }
        if (exists(assets->games->path))
            dirs.emplace_back(assets->games->path);

        for (const auto &gamedir: dirs) {
            FsSys::ReFs::DirectoryFileAccess directory{gamedir};
            if (!directory)
                continue;

            NX_ASSERT(is_directory(gamedir));
            if (!AddTypedGame(gamedir)) {
                if (const auto &_roms{directory.ListAllFiles()}; !_roms.empty()) {
                    for (const auto &gamefiles: _roms) {
                        if (const auto path{FsSys::GetFullPath(gamefiles)}) {
                            if (!AddTypedGame(*path))
                                AsyncLogger::Info("The file {} was filtered out from the selection", FsSys::GetPathStr(*path));
                        } else {
                            throw exception{"The file {} does not exist or is a broken symlink", FsSys::GetPathStr(gamefiles)};
                        }
                    }
                }
            }
        }
    }

    bool GamesLists::AddTypedGame(const FsSys::FsPath &gamefiles) {
        GamePathType _type{};
        if (is_directory(gamefiles)) {
            if (Loaders::IsApplicationDirectory(gamefiles))
                _type = GamePathType::ApplicationDirectory;
        } else {
            if (gamefiles.extension() == ".xci")
                _type = GamePathType::Card;
            else
                _type = GamePathType::Shop;
        }
        if (_type != GamePathType::Unrecognized && IsFormatAllowed(_type)) {
            gamespaths.emplace_back(_type, gamefiles);
            return true;
        }

        return {};
    }
    bool GamesLists::IsFormatAllowed(const GamePathType type) {
        if (!GetContext()->settings->enablensps)
            if (type == GamePathType::Shop)
                return {};

        return true;
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
