#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <loaders/homefs.h>

namespace FastNx::Loaders {
    bool IsHomebrewFsDirectory(const FsSys::FsPath &dirfs) {
        NX_ASSERT(is_directory(dirfs));
        const FsSys::ReFs::EditableDirectory directory{dirfs};

        const auto content{directory.ListAllFiles()};
        I32 version{};
        if (Contains(content, {"version.txt"}))
            version = directory.ListAllFiles().size();
        // We're not reading the version for now, but we expect a value greater than 0

        I32 ishomefs{version > 100};
        switch (version) {
            case 102:
                if (Contains(content, {"exefs/main.npdm", "exefs/main"}))
                    ishomefs++;
                [[fallthrough]];
            case 101:
                if (Contains(content, {"home.json"}))
                    ishomefs++;
                break;
            default:
                return {};
        }
        return ishomefs;
    }
}
