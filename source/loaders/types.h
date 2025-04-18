#pragma once

#include <fs_sys/types.h>

namespace FastNx::Kernel::Types {
    class KProcess;
}

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
            backing(file), type(_type), isloaded(loaded) {}

        virtual void LoadApplication(std::shared_ptr<Kernel::Types::KProcess> &kprocess) = 0;
        virtual std::vector<U8> GetLogo() = 0;
        virtual U64 GetTitleId() = 0;

        const FsSys::VfsBackingFilePtr backing;
        const AppType type;
        LoaderStatus status{};
    protected:
        bool &isloaded;
        void Finish() const;
    };
    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file);

    std::string AppTypeToString(AppType type);
    std::string GetLoaderPrettyString(const std::shared_ptr<AppLoader> &loader);
}
