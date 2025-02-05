#include <cassert>
#include <unistd.h>

#include <fs_sys/refs/editable_directory.h>
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
        const FsSys::ReFs::EditableDirectory directory{target, true};
        assert(fchdir(directory.descriptor) == 0);
    }

    Assets::Assets() {
        const auto _pcwd{std::filesystem::current_path()};
        assert(!_pcwd.empty());
        if (const auto target{"com/callsvc/fastnx"_fs}; !ContainsPath(_pcwd, target)) {
            MoveProcess(target);
        }

        directory = std::filesystem::current_path();
        games = directory / "games";

        FsSys::ReFs::EditableDirectory _games{games, true};
    }

    void Assets::Destroy() {
        gamesLists->assets.reset();
    }

    void Assets::Initialize() {
        gamesLists.emplace(shared_from_this());
    }
}
