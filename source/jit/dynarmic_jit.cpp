#include <boost/container/small_vector.hpp>
#include <common/container.h>
#include <jit/scoped_signal_handler.h>
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
    void PrintArm(const std::span<U64> &armlist) {
        for (const auto [index, regval]: armlist | std::ranges::views::enumerate) {
            if (index <= 8)
                AsyncLogger::Puts("R{}, Value: {:#X} ", index, regval);
            if (index == 97 || index == 99)
                AsyncLogger::Puts("{}: Value: {:#X} ", index == 97 ? "SP" : index == 99 ? "PC" : "?", regval);
        }
        AsyncLogger::Puts("\n");
    }

    void JitDynarmicController::Run(JitThreadContext &context) {
        SetRegisters(context.arm_reglist);

        if (!jitcore->GetSP()) {
            jitcore->SetPC(pagination->GetPage(context.pc_counter));
            jitcore->SetSP(pagination->GetPage(context.stack));
        }

        for (U64 counter{}; counter < 100; counter++) {
            if (!callbacks.GetTicksRemaining())
                callbacks.ticksleft += 500 * std::max(counter, 1UL);

            if (jitcore && initialized) {
                ScopedSignalHandler installactions;
                jitcore->ClearExclusiveState();
                if (jitcore->Run() == Dynarmic::HaltReason::MemoryAbort)
                    PrintArm(ToSpan(context.arm_reglist));
            }
            GetRegisters(ToSpan(context.arm_reglist));
        }
    }

    void JitDynarmicController::Initialize(void *excepttls, void *usertls) {
        // jitconfigs.page_table = pagination->table.data();
        // jitconfigs.page_table_address_space_bits = 39;

        tpidr_el0 = reinterpret_cast<U64>(excepttls);
        tpidrro_el0 = reinterpret_cast<U64>(usertls);

        callbacks.jitctrl = shared_from_this();
        jitconfigs.callbacks = &callbacks;
        jitcore = std::make_unique<Dynarmic::A64::Jit>(jitconfigs);
        jitcore->ClearCache();

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
        NX_ASSERT(regit < regslist.size() && regit < PackedRegistersListSize);
    }

    void JitDynarmicController::SetRegisters(const std::span<U64> &regslist) {
        U64 regit{};
        for (; regit < 31; ++regit)
            jitcore->SetRegister(regit, regslist[regit]);
        const std::span vectors{regslist.subspan(31, 64)};
        for (const auto &[index, simdpair]: std::ranges::views::enumerate(vectors | std::views::chunk(2))) {
            jitcore->SetVector(index, {simdpair[0], simdpair[1]});
        }
        regit += vectors.size();

        jitcore->SetFpcr(regslist[regit++]);
        jitcore->SetFpsr(regslist[regit++]);
        jitcore->SetPC(regslist[regit++]);
        jitcore->SetPstate(regslist[regit++]);
        jitcore->SetSP(regslist[regit++]);
    }
}
