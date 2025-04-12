#include <horizon/nx_apps.h>

namespace FastNx::Horizon {
    std::map<U64, std::vector<U8>> GetAppsPublicLogo(const std::shared_ptr<SwitchNs> &swnx) {
        std::map<U64, std::vector<U8>> logos;
        for (const auto &loader: swnx->loaders) {
            const auto applogo{loader->GetLogo()};
            logos.emplace(loader->GetTitleId(), std::move(applogo));
        }
        return logos;
    }
}