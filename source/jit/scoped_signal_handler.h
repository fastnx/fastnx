#pragma once

#include <common/async_logger.h>
namespace FastNx::Jit {
    class ScopedSignalHandler {
    public:
        ScopedSignalHandler();
        ~ScopedSignalHandler();
    private:
        static void InstallSignalHandler(I32 signal);
    };
}