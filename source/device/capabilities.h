#pragma once

#include <common/types.h>
namespace FastNx::Device {
    std::pair<std::string, I32> GetArchAspects();

    U64 GetCoresCount();
    U64 GetHostPageSize();
    void LockAllMapping();

    void SetCore(U32 coreid);
}
