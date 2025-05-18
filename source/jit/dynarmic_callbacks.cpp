#include <mutex>
#include <common/async_logger.h>
#include <jit/dynarmic_callbacks.h>

namespace FastNx::Jit {
    U8 DynarmicCallbacks::MemoryRead8(const Dynarmic::A64::VAddr vaddr) {
        const auto *result{reinterpret_cast<U8 *>(vaddr)};
        return ptable->Read<U8>(result);
    }

    Dynarmic::A64::Vector DynarmicCallbacks::MemoryRead128(const Dynarmic::A64::VAddr vaddr) {
        return {MemoryRead64(vaddr), MemoryRead64(vaddr + sizeof(U64))};
    }
    void DynarmicCallbacks::MemoryWrite8(const Dynarmic::A64::VAddr vaddr, const U8 value) {
        auto *result{reinterpret_cast<U8 *>(vaddr)};
        *result = value;
    }
    void DynarmicCallbacks::MemoryWrite128(const Dynarmic::A64::VAddr vaddr, const Dynarmic::A64::Vector value) {
        MemoryWrite64(vaddr, value[0]);
        MemoryWrite64(vaddr + sizeof(U64), value[1]);
    }
    void DynarmicCallbacks::InterpreterFallback(Dynarmic::A64::VAddr pc, size_t num_instructions) {
        std::terminate();
    }

    void DynarmicCallbacks::CallSVC(U32 swi) {
        AsyncLogger::Info("SYSTEM CALL");
        Runtime::SpinLock mutex;
        std::unique_lock lock{mutex};
        // ReSharper disable once CppDFAEndlessLoop
        while (true) {
            std::unique_lock _lock{mutex};
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
}
