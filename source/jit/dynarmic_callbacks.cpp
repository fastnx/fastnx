#include <mutex>

#include <fmt/chrono.h>
#include <common/async_logger.h>
#include <jit/dynarmic_callbacks.h>
#include <jit/dynarmic_jit.h>

#include <kernel/svc/syscall.h>

namespace FastNx::Jit {
    U8 DynarmicCallbacks::MemoryRead8(const Dynarmic::A64::VAddr vaddr) {
        if (ValidateMemoryAccess(vaddr, 1))
            return ptable->Read<U8>(reinterpret_cast<U8 *>(vaddr));
        return {};
    }

    Dynarmic::A64::Vector DynarmicCallbacks::MemoryRead128(const Dynarmic::A64::VAddr vaddr) {
        return {MemoryRead64(vaddr), MemoryRead64(vaddr + 8)};
    }
    void DynarmicCallbacks::MemoryWrite8(const Dynarmic::A64::VAddr vaddr, const U8 value) {
        if (ValidateMemoryAccess(vaddr, 1))
            ptable->Write<U8>(reinterpret_cast<U8 *>(vaddr), value);
    }
    void DynarmicCallbacks::MemoryWrite128(const Dynarmic::A64::VAddr vaddr, const Dynarmic::A64::Vector value) {
        MemoryWrite64(vaddr, value.front());
        MemoryWrite64(vaddr + 8, value.back());
    }
    void DynarmicCallbacks::InterpreterFallback(Dynarmic::A64::VAddr pc, U64 instructions) {
        std::terminate();
    }

    void DynarmicCallbacks::CallSVC(const U32 syscall) {
        std::unique_lock lock{mutex};

        AsyncLogger::Info("System call number {} occurred at {}", syscall, std::chrono::system_clock::now());
        HosThreadContext armstate;

        parent->GetContext(armstate);
        for (bool dispatched{}; !dispatched; ) {
            if (Kernel::Svc::Syscall64(syscall, armstate))
                dispatched = true;
            if (!dispatched)
                std::unique_lock endless{mutex};

            parent->SetContext(armstate);
        }
    }

    void DynarmicCallbacks::ExceptionRaised(Dynarmic::A64::VAddr pc, Dynarmic::A64::Exception exception) {
        std::terminate();
    }
    void DynarmicCallbacks::AddTicks(const U64 ticks) {
        if (ticks > ticksleft) {
            ticksleft = 0;
            return;
        }
        ticksleft -= ticks;
    }

    U64 DynarmicCallbacks::GetTicksRemaining() {
        return ticksleft;
    }
    U64 DynarmicCallbacks::GetCNTPCT() {
        return {};
    }

    constexpr auto AbortReason{Dynarmic::HaltReason::MemoryAbort};
    bool DynarmicCallbacks::ValidateMemoryAccess(const Dynarmic::A64::VAddr vaddr, const U64 size) const {
        auto *result{reinterpret_cast<U8 *>(vaddr)};
        const auto [attribute, _]{ptable->Contains(result, size)};
        if (attribute == PageAttributeType::Mapped)
            return true;

        const auto &jit{parent->dyn64};
        jit->DumpDisassembly();
        jit->HaltExecution(AbortReason);
        return {};
    }
}
