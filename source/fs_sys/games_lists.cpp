#include <cassert>
#include <functional>

#include <common/container.h>
#include <fs_sys/assets.h>
#include <fs_sys/games_lists.h>

namespace FastNx::FsSys {
    GameLists::GameLists(const std::shared_ptr<Assets> &_assets) : assets(_assets) {
        gamesLists.push_back(assets->games);
        for (const auto &defaultDir: ArrayOf("~/.local/fastnx/games", "~/.fastnx/games", "~/Documents/games")) {
            gamesLists.emplace_back(defaultDir);
        }
        std::function<void(const FsPath &)> FindGamesPaths = [&](const FsPath &path) {
            for (std::filesystem::directory_iterator entry{path}; IsEmpty(entry); ++entry) {
                auto filepath{entry->path()};
                if (is_symlink(filepath))
                    filepath = read_symlink(filepath);

                if (!exists(filepath))
                    assert(0);

                if (is_regular_file(filepath)) {
                    gamespaths.emplace_back(filepath);
                } else if (is_directory(filepath)) {
                    FindGamesPaths(filepath);
                }
            }
        };

        for (const auto &gameDir: gamesLists) {
            if (!exists(gameDir))
                continue;
            FindGamesPaths(gameDir);
        }
    }
}
