#pragma once

#include <common/types.h>
#include <kernel/types.h>
namespace FastNx::Kernel::Memory {
    constexpr auto SwitchMemorySize{4_GBYTES};

    class DeviceLppd4 {
    public:
        DeviceLppd4();
        ~DeviceLppd4();

        MemoryDescriptor GetDescriptor(U64 baseAddr, U64 size) const;
    private:
        U64 size{SwitchMemorySize};
        U8 *hostptr{nullptr};
    };
}