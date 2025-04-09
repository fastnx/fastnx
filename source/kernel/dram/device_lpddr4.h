#pragma once

#include <common/types.h>
namespace FastNx::Kernel::Dram {
    constexpr auto SwitchMemorySize{4_GBYTES};
    class DeviceLppd4 {
    public:
        DeviceLppd4();
        ~DeviceLppd4();
        U64 size{SwitchMemorySize};
        U8 *device{};
    };
}