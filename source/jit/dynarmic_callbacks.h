#pragma once
#include <dynarmic/interface/A64/config.h>

#include <jit/page_table.h>

namespace FastNx::Jit {
    class JitDynarmicController;

    class DynarmicCallbacks final : public Dynarmic::A64::UserCallbacks {
    public:
        U8 MemoryRead8(Dynarmic::A64::VAddr vaddr) override;
        Dynarmic::A64::Vector MemoryRead128(Dynarmic::A64::VAddr vaddr) override;

        U16 MemoryRead16(const Dynarmic::A64::VAddr vaddr) override {
            return static_cast<U16>(MemoryRead8(vaddr)) | static_cast<U16>(MemoryRead8(vaddr + 1)) << 8;
        }
        U32 MemoryRead32(const Dynarmic::A64::VAddr vaddr) override {
            return static_cast<U32>(MemoryRead16(vaddr)) | static_cast<U32>(MemoryRead16(vaddr + 2)) << 16;
        }
        U64 MemoryRead64(const Dynarmic::A64::VAddr vaddr) override {
            return static_cast<U64>(MemoryRead32(vaddr)) | static_cast<U64>(MemoryRead32(vaddr + 4)) << 32;
        }

        void MemoryWrite8(Dynarmic::A64::VAddr vaddr, U8 value) override;
        void MemoryWrite16(const Dynarmic::A64::VAddr vaddr, const U16 value) override {
            MemoryWrite8(vaddr, value);
            MemoryWrite8(vaddr + 1, value >> 8);
        }
        void MemoryWrite32(const Dynarmic::A64::VAddr vaddr, const U32 value) override {
            MemoryWrite16(vaddr, value);
            MemoryWrite16(vaddr + 2, value >> 16);
        }

        void MemoryWrite64(const Dynarmic::A64::VAddr vaddr, const U64 value) override {
            MemoryWrite32(vaddr, value);
            MemoryWrite32(vaddr + 4, value >> 32);
        }

        void MemoryWrite128(Dynarmic::A64::VAddr vaddr, Dynarmic::A64::Vector value) override;
        void InterpreterFallback(Dynarmic::A64::VAddr pc, U64 instructions) override;
        void CallSVC(U32 syscall) override;
        void ExceptionRaised(Dynarmic::A64::VAddr pc, Dynarmic::A64::Exception exception) override;
        void AddTicks(U64 ticks) override;
        U64 GetTicksRemaining() override;
        U64 GetCNTPCT() override;

        bool Validate(Dynarmic::A64::VAddr vaddr, U64 size, bool read) const;

        U64 ticksleft{};
        std::shared_ptr<PageTable> ptable;
        std::shared_ptr<JitDynarmicController> jitctrl;
    };
}
