#include <common/exception.h>
#include <loaders/nsp_es.h>
#include <loaders/types.h>


auto ProbApplicationFilename(const FastNx::FsSys::VfsBackingFilePtr &file) {
    if (is_directory(file->path))
        return FastNx::Loaders::AppType::GameFilesystem;
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
    template <typename T> requires (std::derived_from<T, AppLoader>)
    AppType GetAppType(const FsSys::VfsBackingFilePtr &file) {
        const auto type{T::CheckFileType(file)};
        return type;
    }


    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file) {
        const auto filenameType{ProbApplicationFilename(file)};
        if (const auto _type{GetAppType<NspEs>(file)}; _type == filenameType)
            return _type;

        throw exception{"The file {} diverges in its format and extension", FsSys::GetPathStr(file)};
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
