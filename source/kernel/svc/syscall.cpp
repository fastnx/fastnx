#include <tsl/robin_map.h>

#include <core/application.h>
#include <debug/process_calltracer.h>
#include <kernel/types/kprocess.h>
#include <kernel/svc/syscall.h>

namespace FastNx::Kernel::Svc {
    const tsl::robin_map<U64, SyscallPrototype> syscall_table = {
        {0x6, QueryMemory}
    };

    bool Syscall(const U32 table, Jit::HosThreadContext &context) {
        static auto application{Core::GetContext()};
        static auto &kernel{application->switchnx->kernel};

        const auto &process{kernel->GetCurrentProcess()};
        static const Debug::ProcessCalltracer tracer{process};

        if (syscall_table.contains(table))
            syscall_table.at(table).operator()({process, process->threads.front()}, context);
        else return {};
        return true;
    }
}
