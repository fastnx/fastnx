#include <fs_sys/refs/editable_directory.h>
#include <loaders/homefs.h>

namespace FastNx::Loaders {
    bool IsHomebrewFsDirectory(const FsSys::FsPath &dirfs) {
        assert(is_directory(dirfs));
        FsSys::ReFs::EditableDirectory directory{dirfs};

        const auto _content{directory.ListAllFiles()};
        I32 version{};
        if (Contains(_content, {"version.txt"_fs}))
            version = 1;
        if (!version)
            return {};

        I32 homebrew{};

        if (Contains(_content, {"home.json"_fs}))
            homebrew++;
        if (Contains(_content, {"exefs/main.npdm"_fs, "exefs/main"_fs}))
            homebrew++;

        return homebrew == 2;
    }
}
