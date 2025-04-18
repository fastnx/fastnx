#include <unistd.h>

#include <common/async_logger.h>
#include <common/exception.h>
#include <core/assets.h>

namespace FastNx::Core {
    auto ContainsPath(const FsSys::FsPath &dest, const FsSys::FsPath &src) {
        bool result{};
        auto dit{dest.begin()};
        auto sit{src.begin()};
        while (dit != dest.end() && sit != src.end())
            if (dit++ == sit++)
                break;
        while (dit != dest.end() && sit != src.end())
            result = dit++ == sit++;
        return result;
    }

    void MoveProcess(const FsSys::FsPath &target) {
        const FsSys::ReFs::DirectoryFileAccess directory{target, true};
        NX_ASSERT(fchdir(directory.descriptor) == 0);
    }

    Assets::Assets() {
        const auto _pcwd{std::filesystem::current_path()};
        NX_ASSERT(!_pcwd.empty());
        if (const FsSys::FsPath target{"com/callsvc/fastnx"}; !ContainsPath(_pcwd, target)) {
            MoveProcess(target);
        }
    }

    void Assets::Destroy() {
        gamesLists->assets.reset();
    }

    std::vector<FsSys::FsPath> Assets::GetAllGames() const {
        std::vector<FsSys::FsPath> result;
        NX_ASSERT(EnumRange(GamePathType::ApplicationDirectory, GamePathType::Card).front());

        for (const auto _type: EnumRange(GamePathType::Homebrew, GamePathType::Shop)) {
            if (const auto _titles{gamesLists->GetAllGamesPaths(_type)}; !_titles.empty())
                std::ranges::copy(_titles, std::back_inserter(result));
        }

        return result;
    }

    void Assets::Initialize() {
        directory.emplace(std::filesystem::current_path());
        games.emplace(directory->path / "games", true);
        keys.emplace(directory->path / "keys", true);
        logs.emplace(directory->path / "logs", true);
        tiks.emplace(directory->path / "tiks", true);

        BuildAsyncLogger(&*logs);

        if (keys->ListAllFiles().empty())
            throw exception{"No keys found on the system"};

        gamesLists.emplace(shared_from_this());
    }
}
