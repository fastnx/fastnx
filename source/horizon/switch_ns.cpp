
#include <common/exception.h>
#include <loaders/nsp_es.h>
#include <horizon/switch_ns.h>

namespace FastNx::Horizon {
    SwitchNs::SwitchNs(const std::shared_ptr<KeySet> &ks) : keys(ks),
        kernel(std::make_shared<Kernel::Kernel>()) {}

    void SwitchNs::LoadApplicationFile(const FsSys::VfsBackingFilePtr &appf) {
        if (!*appf)
            return;
        bool isLoaded{};
        application = [&] -> std::shared_ptr<Loaders::AppLoader> {
            switch (Loaders::GetApplicationType(appf)) {
                case Loaders::AppType::NspEs:
                    return std::make_shared<Loaders::NspEs>(appf, keys, isLoaded);
                default:
                    return nullptr;
            }
        }();

        if (!isLoaded)
            throw exception{"Could not load the ROM due to: {}", GetLoaderPrettyString(application)};
        NX_ASSERT(isLoaded);
    }
}
