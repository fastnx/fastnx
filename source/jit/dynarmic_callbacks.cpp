#include <mutex>

#include <fmt/chrono.h>
#include <common/async_logger.h>
#include <jit/dynarmic_callbacks.h>
#include <jit/dynarmic_jit.h>

namespace FastNx::Jit {
    U8 DynarmicCallbacks::MemoryRead8(const Dynarmic::A64::VAddr vaddr) {
        if (Validate(vaddr, 1, true))
            return ptable->Read<U8>(reinterpret_cast<U8 *>(vaddr));
        return {};
    }

    Dynarmic::A64::Vector DynarmicCallbacks::MemoryRead128(const Dynarmic::A64::VAddr vaddr) {
        return {MemoryRead64(vaddr), MemoryRead64(vaddr + 8)};
    }
    void DynarmicCallbacks::MemoryWrite8(const Dynarmic::A64::VAddr vaddr, const U8 value) {
        if (Validate(vaddr, 1, false))
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
        Runtime::SpinLock mutex;
        std::unique_lock lock{mutex};

        AsyncLogger::Info("System call number {} occurred at {}", syscall, std::chrono::system_clock::now());
        std::array<U64, 102> svclist;
        jitctrl->GetRegisters(svclist);

        // ReSharper disable once CppDFAEndlessLoop
        for (; ;) {
            logger->FlushBuffers();
            std::unique_lock inner{mutex};
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

    bool DynarmicCallbacks::Validate(const Dynarmic::A64::VAddr vaddr, const U64 size, [[maybe_unused]] bool read) const {
        auto *result{reinterpret_cast<U8 *>(vaddr)};
        if (!ptable->Contains(result, size)) {
            if (result <= static_cast<U8 *>(ptable->table.back()))
                jitctrl->jitcore->HaltExecution(Dynarmic::HaltReason::MemoryAbort);
            else
                return false;
        }
        return true;
    }
}
