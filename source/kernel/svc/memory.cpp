#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>
#include <kernel/kernel.h>
#include <kernel/svc/syscall.h>


namespace FastNx::Kernel::Svc {
    // https://switchbrew.org/wiki/Rtld
    void QueryMemory(const SyscallParameters &svcblock, Jit::HosThreadContext &context) {
        const auto &memory{svcblock.process->memory};

        const auto *address{memory->table->GetTable(reinterpret_cast<U8 *>(context.X2))};
        auto *output{reinterpret_cast<U8 *>(context.X0)};

        if (const auto meminfo{memory->QueryMemory(address)}) {
            *reinterpret_cast<Memory::MemoryInfo *>(output) = *meminfo;
        }
        context.W1 = {};
        context.W0 = std::to_underlying(Result::Success);
    }
}
