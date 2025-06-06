#include <boost/container/small_vector.hpp>
#include <common/container.h>

#include <kernel/types/kprocess.h>
#include <kernel/kernel.h>

#include <jit/scoped_signal_handler.h>
#include <jit/dynarmic_jit.h>
namespace FastNx::Jit {

    JitDynarmicController::JitDynarmicController(
        const std::shared_ptr<Kernel::Types::KProcess> &ownerproc) : process(ownerproc) {

        callbacks.ptable = process->kernel.pagetable;
#if !NDEBUG
        jitconfigs.check_halt_on_memory_access = true;
        jitconfigs.detect_misaligned_access_via_page_table = 8 | 16 | 32 | 64 | 128;
#endif
        jitconfigs.tpidr_el0 = &tpidr_el0;
        jitconfigs.tpidrro_el0 = &tpidrro_el0;
        jitconfigs.absolute_offset_page_table = true;
    }
    void PrintArm(const HosThreadContext &context) {
        for (const auto [index, value]: std::ranges::views::enumerate(context.gprlist | std::views::drop(8)))
            AsyncLogger::Puts("R{}, Value: {:X} ", index, value);

        AsyncLogger::Puts("SP, Value: {:X} ", context.sp);
        AsyncLogger::Puts("PC, Value: {:X} ", context.pc);
        AsyncLogger::Puts("\n");
    }

    void JitDynarmicController::Run(JitThreadContext &context) {
        SetRegisters(context.arm_reglist);

        if (!jitcore->GetSP()) {
            jitcore->SetPC(reinterpret_cast<U64>(context.pc_counter));
            jitcore->SetSP(reinterpret_cast<U64>(context.stack));
        }

        for (U64 counter{}; counter < 100; counter++) {
            if (!callbacks.GetTicksRemaining())
                callbacks.ticksleft += 10000;

            if (jitcore && initialized) {
                ScopedSignalHandler installactions;
                jitcore->ClearExclusiveState();
                if (jitcore->Run() == Dynarmic::HaltReason::MemoryAbort)
                    PrintArm(context.arm_reglist);
            }
        }
        GetRegisters(context.arm_reglist);
    }

    void JitDynarmicController::Initialize(void *excepttls, void *usertls) {
        /*
        jitconfigs.page_table = reinterpret_cast<void **>(process->kernel.pagetable->table.data());
        jitconfigs.absolute_offset_page_table = true;
        jitconfigs.page_table_address_space_bits = callbacks.ptable->pagetablewidth;
        jitconfigs.page_table_pointer_mask_bits = PageTable::PageAttrBitsCount;
        */

        tpidr_el0 = reinterpret_cast<U64>(excepttls);
        tpidrro_el0 = reinterpret_cast<U64>(usertls);

        callbacks.jitctrl = shared_from_this();
        jitconfigs.callbacks = &callbacks;
        jitcore = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);
        jitcore->ClearCache();

        initialized = true;
    }

    void JitDynarmicController::GetRegisters(HosThreadContext &jitregs) {
        boost::container::small_vector<U64, 31> regsvalues;
        for (const auto value: jitcore->GetRegisters()) {
            regsvalues.emplace_back(value);
        }

        Copy(jitregs.gprlist, regsvalues);
        jitregs.sp = jitregs.gprlist[29];
        jitregs.pc = jitregs.gprlist[30];

        jitregs.floats = jitcore->GetVectors();

        jitregs.fpcr = jitcore->GetFpcr();
        jitregs.fpsr = jitcore->GetFpsr();
        jitregs.pstate = jitcore->GetPstate();
    }

    void JitDynarmicController::SetRegisters(const HosThreadContext &jitregs) {
        jitcore->SetRegisters(jitregs.gprlist);

        auto vectors{jitcore->GetVectors()};
        for (const auto &[index, floats]: std::views::enumerate(jitregs.floats)) {
            Copy(vectors[index], floats);
        }
        jitcore->SetVectors(vectors);
        jitcore->SetSP(jitregs.sp);
        jitcore->SetPC(jitregs.pc);

        jitcore->SetPstate(jitregs.pstate);
        jitcore->SetFpsr(jitregs.fpsr);
        jitcore->SetFpcr(jitregs.fpcr);
    }
}
