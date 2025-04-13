#include <algorithm>
#include <ranges>

#include <common/exception.h>
#include <loaders/nsp_es.h>
#include <horizon/switch_ns.h>

#include <fs_sys/refs/huge_file.h>

namespace FastNx::Horizon {
    SwitchNs::SwitchNs(const std::shared_ptr<KeySet> &ks) : keys(ks),
        kernel(std::make_shared<Kernel::Kernel>()) {}

    void SwitchNs::LoadApplicationFile(const FsSys::FsPath &apppath) {
        const auto apploader{std::ranges::find_if(loaders, [&](const auto &loader) {
            if (FsSys::GetPathStr(loader->backing) == FsSys::GetPathStr(apppath))
                return true;
            return false;
        })};

        if (apploader == loaders.end())
            throw exception{"Could not load the ROM due to: {}", GetLoaderPrettyString(application)};
        application = *apploader;
    }

    void SwitchNs::GetLoaders(const std::vector<FsSys::FsPath> &apps) {
        bool isloaded{};
        for (const auto &loadable: apps) {
            if (std::filesystem::file_size(loadable) < 1_KBYTES)
                continue;

            const auto storage{std::make_shared<FsSys::ReFs::HugeFile>(loadable)};
            const auto loader = [&] -> std::shared_ptr<Loaders::AppLoader> {
                switch (Loaders::GetApplicationType(storage)) {
                    case Loaders::AppType::NspEs:
                        return std::make_shared<Loaders::NspEs>(storage, keys, isloaded);
                    default:
                        return nullptr;
                    }
            }();
            if (isloaded)
                loaders.emplace_back(loader);
        }
    }
}
