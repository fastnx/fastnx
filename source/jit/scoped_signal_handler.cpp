#include <signal.h>

#include <jit/scoped_signal_handler.h>
namespace FastNx::Jit {
    std::map<I32, struct sigaction> oldsigs;

    void ScopedSignalHandler::InstallSignalHandler(const I32 signal) {
        auto SignalHandler = [](const I32 signum, siginfo_t *siginfo, void *context) {
            const auto *ucontext{static_cast<ucontext_t *>(context)};
            fmt::println("Stack Pointer: {:#X}", static_cast<U64>(ucontext->uc_mcontext.gregs[REG_RSP]));
            oldsigs[signum].sa_sigaction(signum, siginfo, context);
        };

        struct sigaction installsig{.sa_flags = SA_SIGINFO | SA_ONSTACK | SA_RESTART};
        installsig.sa_sigaction = SignalHandler;
        sigemptyset(&installsig.sa_mask);

        sigaction(signal, &installsig, &oldsigs[signal]);
    }

    ScopedSignalHandler::ScopedSignalHandler() {
        InstallSignalHandler(SIGSEGV);
        InstallSignalHandler(SIGABRT);
    }
    ScopedSignalHandler::~ScopedSignalHandler() {
        for (const auto &[signal, handler]: oldsigs)
            sigaction(signal, &handler, nullptr);
    }
}