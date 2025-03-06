#pragma once
#include <loaders/types.h>
#include <fs_sys/types.h>

namespace FastNx::Horizon {
    class SwitchNs {
    public:
        SwitchNs() = default;

        void LoadApplicationFile(const FsSys::VfsBackingFilePtr &appf);

        std::shared_ptr<Loaders::AppLoader> application;
    };
}
