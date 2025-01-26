#include <cassert>
#include <unistd.h>
#include <fs_sys/ssd/editable_directory.h>
#include <fs_sys/assets.h>

namespace FastNx::FsSys {
    auto ContainsPath(const FsPath &dest, const FsPath &src) {
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

    void MoveProcess(const FsPath &target) {
        const SSD::EditableDirectory directory{target, true};
        assert(fchdir(directory.descriptor) == 0);
    }

    Assets::Assets() {
        if (const FsPath target{"com/callsvc/fastnx"}; !ContainsPath(std::filesystem::current_path(), target))
            MoveProcess(target);

        directory = std::filesystem::current_path();
        games = directory / "games";
    }

    Assets::~Assets() {
        gLists->assets.reset();
    }
    void Assets::Initialize() {
        gLists.emplace(shared_from_this());
    }
}
