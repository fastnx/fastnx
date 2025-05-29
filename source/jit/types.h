#pragma once

#include <common/types.h>
namespace FastNx::Jit {
    struct HosThreadContext {
        union GuestRegister {
            explicit operator U64() const {
                return x;
            }
            U64 x;
            U32 w;
        };
        union {
            struct {
                GuestRegister r0, r1, r2;
            };
            std::array<U64, 31> gprlist{};
        };
        std::array<std::array<U64, 2>, 32> floats{};
        U32 fpcr{};
        U32 fpsr{};
        U64 pstate{};

        U64 pc;
        U64 sp;
    };

    struct JitThreadContext {
        void *pc_counter{nullptr};
        void *stack{nullptr};

        HosThreadContext arm_reglist;
    };

    class JitCallBack {
    public:
        virtual ~JitCallBack() = default;

        virtual void Initialize(void *excepttls, void *usertls) = 0;
        virtual void Run(JitThreadContext &context) = 0;

        virtual void GetRegisters(HosThreadContext &jitregs) = 0;
        virtual void SetRegisters(const HosThreadContext &jitregs) = 0;

        bool initialized{};
    };
}