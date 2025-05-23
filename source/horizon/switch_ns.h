#pragma once
#include <loaders/types.h>
#include <fs_sys/types.h>

#include <kernel/kernel.h>
#include <horizon/process_loader.h>
#include <horizon/key_set.h>

namespace FastNx::Horizon {
    class SwitchNs : public std::enable_shared_from_this<SwitchNs> {
    public:
        explicit SwitchNs(const std::shared_ptr<KeySet> &ks);

        void LoadApplicationFile(const FsSys::FsPath &apppath);
        void GetLoaders(const std::vector<FsSys::FsPath> &apps);

        std::shared_ptr<KeySet> keys;
        std::shared_ptr<Loaders::AppLoader> loader;

        std::vector<std::shared_ptr<Loaders::AppLoader>> loaders;
        std::shared_ptr<Kernel::Kernel> kernel;

        std::optional<ProcessLoader> procloader;
    };
}
