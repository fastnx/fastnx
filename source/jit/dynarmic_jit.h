#pragma once
#include <dynarmic/interface/A64/a64.h>

#include <jit/types.h>
#include <jit/dynarmic_callbacks.h>
#include <jit/page_table.h>


namespace FastNx::Jit {
    class JitDynarmicController final : public JitCallBack, public std::enable_shared_from_this<JitDynarmicController> {
    public:
        explicit JitDynarmicController(const std::shared_ptr<Kernel::Types::KProcess> &process);
        void Run(JitThreadContext &context) override;
        void Initialize(void *excepttls, void *usertls) override;

        void GetRegisters(const std::span<U64> &regslist) override;
        void SetRegisters(const std::span<U64> &regslist) override;

        Dynarmic::A64::UserConfig jitconfigs{};
        DynarmicCallbacks callbacks;
        std::unique_ptr<Dynarmic::A64::Jit> jitcore;

        // https://developer.arm.com/documentation/ddi0601/2025-03/AArch64-Registers/TPIDR-EL0--EL0-Read-Write-Software-Thread-ID-Register
        U64 tpidrro_el0{}, tpidr_el0{};
        std::shared_ptr<PageTable> pagination;
    };
}
