#include <cassert>
#include <fs_sys/types.h>
namespace FastNx::FsSys {
    bool IsInsideOf(const FsPath &path, const FsPath &is) {
        assert(exists(path) && exists(is));
        auto dir{path.begin()};
        for (auto it{is.begin()}; dir != path.end() && it != is.end(); ++dir, ++it) {
            if (*it != *dir)
                return {};
        }
        return true;
    }

    bool IsAPfs0File(const VfsBackingFilePtr &pfs0) {
        return pfs0->Read<U32>() == ConstMagicValue<U32>("PFS0");
    }
}
