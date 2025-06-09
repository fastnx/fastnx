#pragma once
#include <dynarmic/interface/A64/a64.h>

#include <jit/types.h>
#include <jit/dynarmic_callbacks.h>

namespace FastNx::Jit {
    class DynarmicJit final : public JitCallBack, public std::enable_shared_from_this<DynarmicJit> {
    public:
        explicit DynarmicJit(const std::shared_ptr<Kernel::Types::KProcess> &runnable);
        void Run(JitThreadContext &context) override;
        void Initialize(void *excepttls, void *usertls) override;

        void GetContext(HosThreadContext &jitregs) override;
        void Reset() override;
        void SetContext(const HosThreadContext &jitregs) override;

        Dynarmic::A64::UserConfig jitconfigs{};
        DynarmicCallbacks callbacks;
        std::unique_ptr<Dynarmic::A64::Jit> dyn64;

        // https://developer.arm.com/documentation/ddi0601/2025-03/AArch64-Registers/TPIDR-EL0--EL0-Read-Write-Software-Thread-ID-Register
        U64 tpidrro_el0{}, tpidr_el0{};
        const std::shared_ptr<Kernel::Types::KProcess> &process;
    };
}
