#include <loaders/types.h>

auto ProbApplicationFilename(const FastNx::FsSys::VfsBackingFilePtr &file) {
    if (const auto &_filePath{file->path}; _filePath.has_extension()) {
        if (const auto _fileExt{_filePath.extension()}; _fileExt == ".nsp") {
            return FastNx::Loaders::AppType::NspEs;
        }
    }
    return FastNx::Loaders::AppType::None;
}

namespace FastNx::Loaders {
    void AppLoader::Finish() const {
        isloaded = type != AppType::None;
        if (isloaded)
            isloaded = status == LoaderStatus::Success;
    }

    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file) {
        const auto firstType{ProbApplicationFilename(file)};
        return firstType;
    }
    std::string AppTypeToString(const AppType type) {
        switch (type) {
            case AppType::NspEs:
                return "NSP";
            default:
                return {};
        }
        std::unreachable();
    }

    std::string GetLoaderPrettyString(const std::shared_ptr<AppLoader> &loader) {
        if (loader->type == AppType::None)
            return "Unknown";
        if (!loader->backing)
            return "File unavailable";
        switch (loader->status) {
            case LoaderStatus::PfsNotFound:
                return "PFS not found";
            default:
                return std::format("{} successfully loaded", AppTypeToString(loader->type));
        }
    }
}
