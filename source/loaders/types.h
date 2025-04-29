#pragma once

#include <fs_sys/types.h>
#include <loaders/application_directory.h>

namespace FastNx::Kernel::Types {
    class KProcess;
}

namespace FastNx::Loaders {
    enum class AppType {
        None,
        NspEs,
        NsoExe,
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

        virtual std::vector<U8> GetLogo() {
            return {};
        }
        virtual U64 GetTitleId() {
            return {};
        }

        const FsSys::VfsBackingFilePtr backing;
        std::shared_ptr<ApplicationDirectory> appdir;

        const AppType type;
        LoaderStatus status{};

        static AppType CheckFileType([[maybe_unused]] const FsSys::VfsBackingFilePtr &file) {
            return AppType::None;
        }
    protected:
        bool &isloaded;
        void Finish() const;
    };
    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file);

    std::string AppTypeToString(AppType type);
    std::string GetLoaderPrettyString(const std::shared_ptr<AppLoader> &loader);
}
