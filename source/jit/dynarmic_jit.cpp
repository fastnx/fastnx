#include <boost/container/small_vector.hpp>
#include <common/container.h>
#include <jit/arm_debug.h>
#include <jit/dynarmic_jit.h>



namespace FastNx::Jit {

    JitDynarmicController::JitDynarmicController(
        const std::shared_ptr<Kernel::Types::KProcess> &process) :
            pagination(std::make_shared<PageTable>(process)) {

        jitconfigs.hook_hint_instructions = true;
        callbacks.ptable = pagination;
#if !NDEBUG
        jitconfigs.check_halt_on_memory_access = true;
        jitconfigs.detect_misaligned_access_via_page_table = 8 | 16 | 32 | 64 | 128;
#endif
        jitconfigs.tpidr_el0 = &tpidr_el0;
        jitconfigs.tpidrro_el0 = &tpidrro_el0;
        jitconfigs.absolute_offset_page_table = true;
    }
    void JitDynarmicController::Run() {
        for (U64 counter{}; counter < 10; counter++) {
            if (!callbacks.GetTicksRemaining())
                callbacks.ticksleft += 500 * std::max(counter, 1UL);

            boost::container::small_vector<U64, 102> regs64(102);
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

    void JitDynarmicController::Initialize(const JitThreadContext &context) {
        // jitconfigs.page_table = pagination->table.data();
        // jitconfigs.page_table_address_space_bits = 39;

        tpidr_el0 = reinterpret_cast<U64>(context.exceptiontls);
        tpidrro_el0 = reinterpret_cast<U64>(context.usertls);

        callbacks.jitctrl = shared_from_this();
        jitconfigs.callbacks = &callbacks;
        jitcore = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);
        jitcore->ClearCache();
        jitcore->SetPC(pagination->GetPage(context.entry));
        jitcore->SetSP(pagination->GetPage(context.stack));

        initialized = true;
    }

    void JitDynarmicController::GetRegisters(const std::span<U64> &regslist) {
        U64 regit{};
        for (; regit < 31; ++regit)
            regslist[regit] = jitcore->GetRegister(regit);

        for (const auto &simdpair: jitcore->GetVectors()) {
            regslist[regit++] = simdpair[0];
            regslist[regit++] = simdpair[1];
        }

        regslist[regit++] = jitcore->GetFpcr();
        regslist[regit++] = jitcore->GetFpsr();
        regslist[regit++] = jitcore->GetPC();
        regslist[regit++] = jitcore->GetPstate();
        regslist[regit++] = jitcore->GetSP();
        NX_ASSERT(regit < regslist.size());
    }
}
