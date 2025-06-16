#pragma once
#include <common/types.h>
namespace FastNx::Core {
    class Application;

    U64 GetCacheDirectorySize();

    void ClearCache(const std::shared_ptr<Application> &application);
    void ExportCache(const std::shared_ptr<Application> &application);
}