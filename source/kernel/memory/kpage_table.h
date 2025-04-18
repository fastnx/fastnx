#pragma once
#include <kernel/types.h>
#include <fs_sys/types.h>

namespace FastNx::Kernel::Memory {
    class KPageTable {
    public:
        explicit KPageTable(Kernel &_kernel) : kernel(_kernel) {}

        void CreateProcessMemory(const FsSys::VfsBackingFilePtr &npdm);

        Kernel &kernel;
        ProcessAddressSpace currpas{};
    };
}
