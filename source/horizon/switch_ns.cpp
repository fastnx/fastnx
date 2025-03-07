#include <cassert>

#include <common/exception.h>
#include <loaders/nsp_es.h>
#include <horizon/switch_ns.h>
void FastNx::Horizon::SwitchNs::LoadApplicationFile(const FsSys::VfsBackingFilePtr &appf) {
    if (!*appf)
        return;

    bool isLoaded{};
    application = [&] -> std::shared_ptr<Loaders::AppLoader> {
        switch (Loaders::GetApplicationType(appf)) {
            case Loaders::AppType::NspEs:
                return std::make_shared<Loaders::NspEs>(appf, isLoaded);
            default:
                return nullptr;
        }
    }();

    if (!isLoaded)
        throw exception{"Could not load the ROM due to: {}", GetLoaderPrettyString(application)};
    assert(isLoaded);
}
