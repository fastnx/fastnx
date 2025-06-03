#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>
#include <kernel/kernel.h>
#include <kernel/svc/syscall.h>


namespace FastNx::Kernel::Svc {
    void QueryMemory(const SyscallParameters &svcblock, Jit::HosThreadContext &context) {
        const auto &memory{svcblock.process->memory};
        const auto &pagination{svcblock.process->kernel.pagetable};

        const auto *address{pagination->GetTable(reinterpret_cast<U8 *>(U64{context.R2}))};
        auto *outblock{pagination->GetTable(reinterpret_cast<U8 *>(U64{context.R0}))};

        if (auto meminfo{memory->QueryMemory(address)}) {
            meminfo->base = reinterpret_cast<void *>(pagination->GetPage(meminfo->base));
            std::construct_at(reinterpret_cast<Memory::MemoryInfo *>(outblock), *meminfo);
        }

        context.R0.W = std::to_underlying(Result::Success);
    }
}
