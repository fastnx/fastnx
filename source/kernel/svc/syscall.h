#pragma once

#include <functional>

#include <kernel/svc/types.h>
namespace FastNx::Kernel::Svc {
    // https://switchbrew.org/wiki/SVC#QueryMemory
    struct SyscallParameters {
        std::shared_ptr<Types::KProcess> process;
        std::shared_ptr<Types::KThread> thread;
    };

    using SyscallPrototype = std::function<void(const SyscallParameters &, Jit::HosThreadContext &)>;

    void QueryMemory(const SyscallParameters &svcblock, Jit::HosThreadContext &context);
    bool Syscall64(U32 table, Jit::HosThreadContext &context);
}