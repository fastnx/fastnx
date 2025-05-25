#pragma once
#include <dynarmic/interface/A64/a64.h>

#include <jit/types.h>
#include <jit/dynarmic_callbacks.h>
#include <jit/page_table.h>

#include <kernel/memory/k_memory.h>

namespace FastNx::Jit {
    class JitDynarmicJitController final : public JitCallBack {
    public:
        explicit JitDynarmicJitController(const std::shared_ptr<Kernel::Memory::KMemory> &jitmemory);
        void Run() override;
        void Initialize(const JitThreadContext &context) override;

        void GetRegisters(const std::span<U64> &jitregs) override;

        Dynarmic::A64::UserConfig jitconfigs{};
        DynarmicCallbacks callbacks;
        std::unique_ptr<Dynarmic::A64::Jit> jitcore;

        // https://developer.arm.com/documentation/ddi0601/2025-03/AArch64-Registers/TPIDR-EL0--EL0-Read-Write-Software-Thread-ID-Register
        U64 tpidrro_el0{}, tpidr_el0{};
        std::shared_ptr<PageTable> pagination;
    };
}
