#pragma once

#include <common/types.h>
namespace FastNx::Jit {
    constexpr auto PackedRegistersListSize{102};
    struct JitThreadContext {
        void *pc_counter{};
        void *stack;

        std::array<U64, PackedRegistersListSize> arm_reglist;
    };

    class JitCallBack {
    public:
        virtual ~JitCallBack() = default;

        virtual void Initialize(void *excepttls, void *usertls) = 0;
        virtual void Run(JitThreadContext &context) = 0;

        virtual void GetRegisters(const std::span<U64> &jitregs) = 0;
        virtual void SetRegisters(const std::span<U64> &jitregs) = 0;

        bool initialized{};
    };
}