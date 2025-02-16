#pragma once

#include <common/types.h>
namespace FastNx::Kernel {
    constexpr auto InitialProcessId{0x51};
    class KernelContext {
    public:
        KernelContext() = default;

        I32 GetPid(U64 prnde);

        I32 processId{InitialProcessId};
        std::mutex _idsMutex;
    };
}
