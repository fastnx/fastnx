#include <algorithm>

#include <common/exception.h>
#include <loaders/nsp_es.h>
#include <loaders/gamefs.h>
#include <horizon/switch_ns.h>

#include <fs_sys/nacp.h>
#include <fs_sys/refs/huge_file.h>


namespace FastNx::Horizon {
    SwitchNs::SwitchNs(const std::shared_ptr<KeySet> &ks) : keys(ks),
        kernel(std::make_shared<Kernel::Kernel>()) {}

    void StartProcess(std::optional<ProcessLoader> &loader) {
        const auto stacksize{loader->npdm->stacksize};
        Kernel::ThreadPriority priority{loader->npdm->priority};
        const auto optiomalcore{loader->npdm->maincore};

        loader->process->Initialize(stacksize, priority, optiomalcore);
    }

    void SwitchNs::LoadApplicationFile(const FsSys::FsPath &apppath) {
        for (const auto &apploader: loaders) {
            if (FsSys::GetPathStr(apploader->backing) == apppath)
                loader = apploader;
        }
        if (!loader)
            return;
        if (loader->status != Loaders::LoaderStatus::Success) {
            throw exception{"Failed to load the application due to: {}", GetLoaderPrettyString(loader)};
        }

        if (const auto exefs{loader->appdir->GetExefs()})
            if (const auto control{exefs->OpenFile("exefs/control.nacp")})
                [[maybe_unused]] FsSys::Nacp nacp(control);

        if (!procloader)
            procloader.emplace(shared_from_this());
        procloader->Load();

        StartProcess(procloader);
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
                    case Loaders::AppType::GameFilesystem: {
                        const auto appdir{std::make_shared<FsSys::ReFs::DirectoryFileAccess>(loadable)};
                        return std::make_shared<Loaders::GameFs>(std::move(appdir), isloaded);
                    }
                    default:
                        return nullptr;
                    }
            }();
            if (isloaded)
                loaders.emplace_back(loader);
        }
    }
}
