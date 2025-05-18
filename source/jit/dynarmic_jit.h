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


        std::shared_ptr<Kernel::Memory::KMemory> memory;
        Dynarmic::A64::UserConfig jitconfigs;
        DynarmicCallbacks callbacks;
        std::unique_ptr<Dynarmic::A64::Jit> jitcore;


        std::shared_ptr<PageTable> pagination;
    };
}
