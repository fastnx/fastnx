#include <jit/dynarmic_jit.h>
namespace FastNx::Jit {

    JitDynarmicJitController::JitDynarmicJitController(
        const std::shared_ptr<Kernel::Memory::KMemory> &jitmemory) :
            jitconfigs{}, pagination(std::make_shared<PageTable>(jitmemory)) {

        callbacks.ptable = pagination;
        jitconfigs.callbacks = &callbacks;
    }
    void JitDynarmicJitController::Run() {
        if (!callbacks.GetTicksRemaining())
            callbacks.ticksleft = 1'000;
        if (jitcore && initialized)
            jitcore->Run();
    }

    void JitDynarmicJitController::Initialize(const JitThreadContext &context) {
        jitconfigs.page_table = pagination->table.data();
        jitconfigs.page_table_address_space_bits = 39;
        jitcore = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);
        /*
        jitcore->SetPC(reinterpret_cast<U64>(context.pc));
        jitcore->SetSP(reinterpret_cast<U64>(context.stack));
        */

        initialized = true;
    }
}
