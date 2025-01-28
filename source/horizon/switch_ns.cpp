#include <loaders/nsp_es.h>
#include <horizon/switch_ns.h>

void FastNx::Horizon::SwitchNs::LoadApplicationFile(const FsSys::VfsBackingFilePtr &appf) {
    application = [&]() -> Loaders::AppLoaderPtr {
        switch (Loaders::GetApplicationType(appf)) {
            case Loaders::AppType::NspEs:
                return std::make_shared<Loaders::NspEs>(appf);
            default:
                return nullptr;
        }
    }();
}
