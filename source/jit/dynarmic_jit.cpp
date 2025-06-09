#include <boost/container/small_vector.hpp>
#include <common/container.h>

#include <kernel/types/kprocess.h>
#include <kernel/kernel.h>

#include <jit/scoped_signal_handler.h>
#include <jit/dynarmic_jit.h>
namespace FastNx::Jit {

    DynarmicJit::DynarmicJit(
        const std::shared_ptr<Kernel::Types::KProcess> &runnable) : process(runnable) {
        callbacks.ptable = process->kernel.pagetable;
#if !NDEBUG
        jitconfigs.check_halt_on_memory_access = true;
        jitconfigs.detect_misaligned_access_via_page_table = 8 | 16 | 32 | 64 | 128;
#endif
        jitconfigs.tpidr_el0 = &tpidr_el0;
        jitconfigs.tpidrro_el0 = &tpidrro_el0;
    }

    void PrintArm(const HosThreadContext &context) {
        auto regit{context.GPRs.begin()};
        const auto regbegin{regit};
        for (; regit != context.GPRs.end(); ++regit)
            AsyncLogger::Puts("R{}, Value: {:X} ", std::distance(regbegin, regit), *regit);

        AsyncLogger::Puts("SP, Value: {:X} ", context.sp);
        AsyncLogger::Puts("PC, Value: {:X} ", context.pc);
        AsyncLogger::Puts("\n");
    }

    void DynarmicJit::Run(JitThreadContext &context) {
        SetContext(context.arm_reglist);
        if (!dyn64 || !initialized)
            return;

        if (!dyn64->GetSP()) {
            dyn64->SetPC(reinterpret_cast<U64>(context.pc_counter));
            dyn64->SetSP(reinterpret_cast<U64>(context.stack));
        }
        bool halted{};
        for (U64 counter{}; counter < 100; counter++) {
            if (!callbacks.GetTicksRemaining())
                callbacks.ticksleft += 100000;

            ScopedSignalHandler installactions;
            if (dyn64->Run() != Dynarmic::HaltReason::MemoryAbort)
                continue;
            halted = true;
            break;
        }
        GetContext(context.arm_reglist);
        if (halted)
            PrintArm(context.arm_reglist);
    }

    void DynarmicJit::Initialize(void *excepttls, void *usertls) {
        /*
        jitconfigs.page_table = reinterpret_cast<void **>(process->kernel.pagetable->table.data());
        jitconfigs.absolute_offset_page_table = true;
        jitconfigs.page_table_address_space_bits = callbacks.ptable->pagetablewidth;
        jitconfigs.page_table_pointer_mask_bits = PageTable::PageAttrBitsCount;
        */

        tpidr_el0 = reinterpret_cast<U64>(excepttls);
        tpidrro_el0 = reinterpret_cast<U64>(usertls);

        jitconfigs.define_unpredictable_behaviour = true;
        callbacks.parent = shared_from_this();
        jitconfigs.callbacks = &callbacks;
        dyn64 = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);
        Reset();

        initialized = true;
    }

    void DynarmicJit::GetContext(HosThreadContext &jitregs) {
        boost::container::small_vector<U64, 31> regsvalues;
        for (const auto value: dyn64->GetRegisters()) {
            regsvalues.emplace_back(value);
        }

        Copy(jitregs.GPRs, regsvalues);
        jitregs.sp = jitregs.GPRs[29];
        jitregs.pc = jitregs.GPRs[30];

        jitregs.floats = dyn64->GetVectors();

        jitregs.fpcr = dyn64->GetFpcr();
        jitregs.fpsr = dyn64->GetFpsr();
        jitregs.pstate = dyn64->GetPstate();
    }

    void DynarmicJit::Reset() {
        dyn64->ClearCache();
        dyn64->Reset();

        dyn64->ClearHalt();
    }

    void DynarmicJit::SetContext(const HosThreadContext &jitregs) {
        dyn64->SetRegisters(jitregs.GPRs);

        std::array<Dynarmic::A64::Vector, 32> vectors;
        for (const auto &[index, floats]: std::views::enumerate(jitregs.floats)) {
            vectors[index] = floats;
        }
        if (!IsEmpty(vectors))
            dyn64->SetVectors(vectors);
        dyn64->SetSP(jitregs.sp);
        dyn64->SetPC(jitregs.pc);

        dyn64->SetPstate(jitregs.pstate);
        dyn64->SetFpsr(jitregs.fpsr);
        dyn64->SetFpcr(jitregs.fpcr);
    }
}
