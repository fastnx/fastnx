#pragma once

#include <kernel/types.h>

namespace FastNx::Kernel::Svc {
    // https://switchbrew.org/wiki/SVC#Result
    enum class MemoryRegion {
        Application,
        Applet,
        SecureSystem,
        NonSecureSystem
    };
#pragma pack(push, 1)
    struct CreateProcessParameter {
        std::array<U8, 12> processname;
        U32 category{}; // 0: regular title, 1: kernel built-in
        U64 titleid;
        U64 codestart; // Aligned address for the first instruction of the executable
        U32 codenumpages;

        union {
            struct {
                bool is64bitinstruction: 1;
                ProcessAddressSpace addrspace: 3;
                bool enbdebug: 1;
                bool enbaslr: 1;
                bool isapplication: 1;
                bool usesecurememory: 1;
                MemoryRegion memoryregion: 3;
                bool optimizememoryallocation: 1; // only allowed in combination with IsApplication
            };
            U32 flagsint;
        };
        U32 resourcelimit;
        U32 systemnumpages;
    };
#pragma pack(pop)
}