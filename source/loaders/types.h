#pragma once

#include <fs_sys/types.h>

namespace FastNx::Loaders {
    enum class AppType {
        None,
        NspEs,
        GameFilesystem
    };

    enum class LoaderStatus {
        Undefined,
        Success,
        PfsNotFound,
    };

    class AppLoader {
    public:
        virtual ~AppLoader() = default;

        explicit AppLoader(const FsSys::VfsBackingFilePtr &file, bool &loaded, const AppType _type) :
            backing(file), type(_type), isloaded(loaded) {
        }
        const FsSys::VfsBackingFilePtr backing;
        const AppType type;
        LoaderStatus status{};

        virtual std::vector<U8> GetLogo() = 0;
        virtual U64 GetTitleId() = 0;
    protected:
        bool &isloaded;
        void Finish() const;
    };
    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file);

    std::string AppTypeToString(AppType type);
    std::string GetLoaderPrettyString(const std::shared_ptr<AppLoader> &loader);
}
