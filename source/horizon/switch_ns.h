#pragma once
#include <loaders/types.h>
#include <fs_sys/types.h>

#include <kernel/kernel.h>
#include <horizon/key_set.h>

namespace FastNx::Horizon {
    class SwitchNs {
    public:
        explicit SwitchNs(const std::shared_ptr<KeySet> &ks);

        void LoadApplicationFile(const FsSys::VfsBackingFilePtr &appf);

        std::shared_ptr<KeySet> keys;
        std::shared_ptr<Loaders::AppLoader> application;
        std::shared_ptr<Kernel::Kernel> kernel;
    };
}
