#pragma once

#include <common/types.h>
namespace FastNx::Jit {
    struct REGW {
        void operator=(const U32 val) {
            value = val;
            upper = {};
        }
        U32 value;
        U32 upper;
    };
    struct HosThreadContext {
        union {
            struct {
                U64 X0, X1, X2, X3, X4, X5, X6, X7;
            };
            struct {
                REGW W0, W1, W2, W3, W4, W5, W6, W7;
            };
            std::array<U64, 31> GPRs{};
        };
        std::array<std::array<U64, 2>, 32> floats{};
        U32 fpcr{};
        U32 fpsr{};
        U64 pstate{};

        U64 pc{};
        U64 sp{};
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
        virtual void Reset() = 0;

        virtual void GetContext(HosThreadContext &jitregs) = 0;
        virtual void SetContext(const HosThreadContext &jitregs) = 0;

        bool initialized{};
    };
}