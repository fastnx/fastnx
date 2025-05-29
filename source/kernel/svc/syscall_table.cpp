#include <functional>
#include <tsl/robin_map.h>

#include <core/application.h>
#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>

namespace FastNx::Kernel::Svc {

    // https://switchbrew.org/wiki/SVC#QueryMemory
    struct SyscallParameters {
        std::shared_ptr<Types::KProcess> process;
        std::shared_ptr<Types::KThread> thread;
    };
    using SyscallPrototype = std::function<void(const SyscallParameters &, const Jit::HosThreadContext &)>;
    static void QueryMemory(const SyscallParameters &svcblock, const Jit::HosThreadContext &context) {
        const auto &memory{svcblock.process->memory};
        const auto &pagination{svcblock.process->kernel.pagetable};

        const auto *address{pagination->GetTable(reinterpret_cast<U8 *>(U64{context.r2}))};
        auto *outblock{pagination->GetTable(reinterpret_cast<U8 *>(U64{context.r0}))};

        if (const auto *meminfo{memory->QueryMemory(address)})
            std::construct_at(reinterpret_cast<Memory::KMemoryBlock *>(outblock), *meminfo);
    }

    const tsl::robin_map<U64, SyscallPrototype> syscall_table = {
        {0x6, QueryMemory}
    };

    void Syscall(const U32 table, const Jit::HosThreadContext &context) {
        static auto application{Core::GetContext()};
        static auto &kernel{application->switchnx->kernel};

        const auto &process{kernel->GetCurrentProcess()};
        if (syscall_table.contains(table))
            syscall_table.at(table).operator()({process, process->threads.front()}, context);

    }
}
