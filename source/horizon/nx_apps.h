#pragma once

#include <map>
#include <horizon/switch_ns.h>
namespace FastNx::Horizon {
    std::map<U64, std::vector<U8>> GetAppsPublicLogo(const std::shared_ptr<SwitchNs> &swnx);
}