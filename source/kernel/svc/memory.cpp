#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>
#include <kernel/kernel.h>
#include <kernel/svc/syscall.h>


namespace FastNx::Kernel::Svc {
    void QueryMemory(const SyscallParameters &svcblock, Jit::HosThreadContext &context) {
        const auto &memory{svcblock.process->memory};

        const auto *address{reinterpret_cast<U8 *>(context.X2)};
        auto *outblock{reinterpret_cast<U8 *>(context.X0)};

        if (auto meminfo{memory->QueryMemory(address)}) {
            std::construct_at(reinterpret_cast<Memory::MemoryInfo *>(outblock), *meminfo);
        }
        context.W1 = {};
        context.W0 = std::to_underlying(Result::Success);
    }
}
