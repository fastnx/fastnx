#pragma once
#include <signal.h>

#include <common/async_logger.h>
namespace FastNx::Jit {
    class ScopedSignalHandler {
    public:
        ScopedSignalHandler() {
            SetSignalHandler(SIGSEGV);
            SetSignalHandler(SIGABRT);
        }
        ~ScopedSignalHandler();
    private:
        static void SetSignalHandler(I32 signal);
    };
    void PrintArm(const std::span<U64> &armlist);
}