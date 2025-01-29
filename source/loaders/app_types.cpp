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
    void AppLoader::_Finish() const {
        isLoaded = type != AppType::None;
        if (isLoaded)
            isLoaded = status == LoaderStatus::Success;
    }

    AppType GetApplicationType(const FsSys::VfsBackingFilePtr &file) {
        const auto firstType{ProbApplicationFilename(file)};
        return firstType;
    }
}
