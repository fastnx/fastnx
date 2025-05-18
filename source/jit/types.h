#pragma once

#include <common/types.h>
namespace FastNx::Jit {
    struct JitThreadContext {
        void *stack{};
        void *pc{};
    };

    class JitCallBack {
    public:
        virtual ~JitCallBack() = default;

        virtual void Initialize(const JitThreadContext &context) = 0;
        virtual void Run() = 0;

        bool initialized{};
    };
}