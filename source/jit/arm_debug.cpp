#include <signal.h>

#include <jit/arm_debug.h>
namespace FastNx::Jit {
    std::map<I32, struct sigaction> oldsigs;

    void GlobalJitHandle(const I32 signal, siginfo_t *siginfo, void *context) {
        const auto *ucontext{static_cast<ucontext_t *>(context)};
        fmt::println("Stack Pointer: {:#x}", static_cast<U64>(ucontext->uc_mcontext.gregs[REG_RSP]));
        oldsigs[signal].sa_sigaction(signal, siginfo, context);
    }
    void ScopedSignalHandler::SetSignalHandler(const I32 signal) {
        struct sigaction installsig{.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART};
        installsig.sa_sigaction = GlobalJitHandle;
        sigemptyset(&installsig.sa_mask);

        sigaction(signal, &installsig, &oldsigs[signal]);
    }

    ScopedSignalHandler::ScopedSignalHandler() {
        SetSignalHandler(SIGSEGV);
        SetSignalHandler(SIGABRT);
    }
    ScopedSignalHandler::~ScopedSignalHandler() {
        for (const auto &[signal, handler]: oldsigs)
            sigaction(signal, &handler, nullptr);
    }

    void PrintArm(const std::span<U64> &armlist) {
        for (const auto [index, regval]: armlist | std::ranges::views::enumerate)
            if (index <= 8)
                AsyncLogger::Puts("R{}, Value: {:#x} ", index, regval);
            else
                AsyncLogger::Puts("{}: Value: {:#x} ", index == 9 ? "SP" : index == 10 ? "PC" : "?", regval);
        AsyncLogger::Puts("\n");
    }
}