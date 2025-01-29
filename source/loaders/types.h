#pragma once

#include <fs_sys/types.h>

namespace FastNx::Loaders {
    enum class AppType {
        None,
        NspEs
    };

    enum class LoaderStatus {
        Undefined,
        Success,
        PfsNotFound,
    };

    class AppLoader {
    public:
        explicit AppLoader(const FsSys::VfsBackingFilePtr &_backing, bool& _isLoaded, const AppType _type) :
            backing(_backing), type(_type), isLoaded(_isLoaded) {
        }
        const FsSys::VfsBackingFilePtr backing;
        const AppType type;
        LoaderStatus status{LoaderStatus::Undefined};

    protected:
        bool& isLoaded;
        void _Finish() const;
    };
    using AppLoaderPtr = std::shared_ptr<AppLoader>;
    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file);
}
