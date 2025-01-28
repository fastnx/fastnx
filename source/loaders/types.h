#pragma once

#include <fs_sys/types.h>

namespace FastNx::Loaders {
    enum class AppType {
        None,
        NspEs
    };

    class AppLoader {
    public:
        explicit AppLoader(const FsSys::VfsBackingFilePtr &_backing, const AppType _type) : backing(_backing),
            type(_type) {
        }
        const FsSys::VfsBackingFilePtr backing;
        const AppType type;
    };
    using AppLoaderPtr = std::shared_ptr<AppLoader>;
    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file);
}
