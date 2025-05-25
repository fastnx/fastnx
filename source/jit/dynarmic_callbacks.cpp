#include <mutex>

#include <fmt/chrono.h>
#include <common/async_logger.h>
#include <jit/dynarmic_callbacks.h>

namespace FastNx::Jit {
    U8 DynarmicCallbacks::MemoryRead8(const Dynarmic::A64::VAddr vaddr) {
        return ptable->Read<U8>(reinterpret_cast<U8 *>(vaddr));
    }

    Dynarmic::A64::Vector DynarmicCallbacks::MemoryRead128(const Dynarmic::A64::VAddr vaddr) {
        return {MemoryRead64(vaddr), MemoryRead64(vaddr + 8)};
    }
    void DynarmicCallbacks::MemoryWrite8(const Dynarmic::A64::VAddr vaddr, const U8 value) {
        ptable->Write<U8>(reinterpret_cast<U8 *>(vaddr), value);
    }
    void DynarmicCallbacks::MemoryWrite128(const Dynarmic::A64::VAddr vaddr, const Dynarmic::A64::Vector value) {
        MemoryWrite64(vaddr, value.front());
        MemoryWrite64(vaddr + 8, value.back());
    }
    void DynarmicCallbacks::InterpreterFallback(Dynarmic::A64::VAddr pc, size_t num_instructions) {
        std::terminate();
    }

    void DynarmicCallbacks::CallSVC(U32 swi) {
        Runtime::SpinLock mutex;
        std::unique_lock lock{mutex};

        AsyncLogger::Info("System call number {} occurred at {}", swi, std::chrono::system_clock::now());
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
