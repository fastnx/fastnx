#pragma once

#include <common/types.h>
namespace FastNx::Kernel::Memory {
    constexpr auto SwitchMemorySize{4_GBYTES};

    class DeviceMemory {
    public:
        DeviceMemory();
        ~DeviceMemory();

        std::span<U8> GetSpan(U64 baseaddr = {}, U64 offset = {}, bool ishost = false) const;
        std::span<U8> InitializeGuestAs(U64 aswidth);
    private:
        U64 size{SwitchMemorySize};
        U8 *hostptr{nullptr};
        U8 *guestptr{nullptr};
    };
}