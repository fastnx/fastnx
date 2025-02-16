#include <fs_sys/refs/editable_directory.h>
#include <loaders/homefs.h>

namespace FastNx::Loaders {
    bool IsHomebrewFsDirectory(const FsSys::FsPath &dirfs) {
        assert(is_directory(dirfs));
        FsSys::ReFs::EditableDirectory directory{dirfs};

        const auto _content{directory.ListAllFiles()};
        I32 version{};
        if (Contains(_content, {"version.txt"_fs}))
            version = directory.ListAllFiles().size(); // We're not reading the version for now, but we expect a value greater than 0
        if (!version)
            return {};

        I32 isHfs{version > 100};
        if (Contains(_content, {"home.json"_fs}))
            isHfs++;
        if (Contains(_content, {"exefs/main.npdm"_fs, "exefs/main"_fs}))
            isHfs += 2;

        return isHfs >= 2;
    }
}
