#include <iostream>
#include <cassert>

#include <common/exception.h>
#include <loaders/nsp_es.h>
#include <horizon/switch_ns.h>
void FastNx::Horizon::SwitchNs::LoadApplicationFile(const FsSys::VfsBackingFilePtr &appf) {
    if (!static_cast<bool>(*appf))
        return;

    bool isLoaded{};
    application = [&] -> Loaders::AppLoaderPtr {
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
