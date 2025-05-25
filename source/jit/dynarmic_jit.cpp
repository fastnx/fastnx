#include <boost/container/small_vector.hpp>
#include <common/container.h>
#include <jit/arm_debug.h>
#include <jit/dynarmic_jit.h>



namespace FastNx::Jit {

    JitDynarmicJitController::JitDynarmicJitController(
        const std::shared_ptr<Kernel::Memory::KMemory> &jitmemory) :
            pagination(std::make_shared<PageTable>(jitmemory)) {

        jitconfigs.hook_hint_instructions = true;
        callbacks.ptable = pagination;
        jitconfigs.callbacks = &callbacks;
#if !NDEBUG
        jitconfigs.check_halt_on_memory_access = true;
        jitconfigs.detect_misaligned_access_via_page_table = 8 | 16 | 32 | 64 | 128;
#endif
        jitconfigs.tpidr_el0 = &tpidr_el0;
        jitconfigs.tpidrro_el0 = &tpidrro_el0;
        jitconfigs.absolute_offset_page_table = true;
    }
    void JitDynarmicJitController::Run() {
        for (U64 counter{}; counter < 10; counter++) {
            if (!callbacks.GetTicksRemaining())
                callbacks.ticksleft += 500 * std::max(counter, 1UL);

            boost::container::small_vector<U64, 12> regs64(12);
            if (jitcore && initialized) {
                ScopedSignalHandler installactions;
                jitcore->ClearExclusiveState();
                switch (jitcore->Run()) {
                    case Dynarmic::HaltReason::MemoryAbort:
                    default: {}
                }
            }
            if (!(counter % 10)) {
                GetRegisters(ToSpan(regs64));
                PrintArm(ToSpan(regs64));
            }
        }
    }

    void JitDynarmicJitController::Initialize(const JitThreadContext &context) {
        // jitconfigs.page_table = pagination->table.data();
        // jitconfigs.page_table_address_space_bits = 39;

        tpidr_el0 = reinterpret_cast<U64>(context.exceptiontls);
        tpidrro_el0 = reinterpret_cast<U64>(context.usertls);

        jitcore = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);
        jitcore->ClearCache();
        jitcore->SetPC(0);
        // jitcore->SetSP(0xBEEFC0DE);

        initialized = true;
    }

    void JitDynarmicJitController::GetRegisters(const std::span<U64> &jitregs) {
        for (U32 regindex{}; regindex < 8; ++regindex)
            jitregs[regindex] = jitcore->GetRegister(regindex);

        jitregs[9] = jitcore->GetSP();
        jitregs[10] = jitcore->GetPC();
    }
}
