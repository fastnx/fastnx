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
    }

    void JitDynarmicJitController::Run() {
        if (!callbacks.GetTicksRemaining())
            callbacks.ticksleft = 100;

        boost::container::small_vector<U64, 12> REGS(12);

        if (jitcore && initialized) {
            ScopedSignalHandler installactions;
            for (U64 counter{}; callbacks.GetTicksRemaining(); counter++) {
                [[unlikely]] if (!(counter % 10)) {
                    GetRegisters(ToSpan(REGS));
                    PrintArm(ToSpan(REGS));
                }
                jitcore->Step();
            }
        }
    }

    void JitDynarmicJitController::Initialize(const JitThreadContext &context) {
        jitconfigs.page_table = pagination->table.data();
        jitconfigs.page_table_address_space_bits = 39;
        jitcore = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);


        // jitcore->SetPC(reinterpret_cast<U64>(context.pc));
        jitcore->SetSP(0xBEEFC0DE);

        initialized = true;
    }

    void JitDynarmicJitController::GetRegisters(const std::span<U64> &jitregs) {
        for (U32 regindex{}; regindex < 8; ++regindex)
            jitregs[regindex] = jitcore->GetRegister(regindex);

        jitregs[9] = jitcore->GetSP();
        jitregs[10] = jitcore->GetPC();
    }
}
