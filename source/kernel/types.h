#pragma once

#include <vector>

#include <common/types.h>
namespace FastNx::Kernel {
    class KAutoObject;
    class Kernel;

    namespace Types {
        class KThread;
        class KProcess;
    }

    using ProcessEntropy = std::array<U64, 4>;

    constexpr auto InitialProcessId{0x51};
    constexpr auto MaximumProcessIds{300};

    struct MemoryDescriptor {
        void *begin{nullptr};
        void *end{nullptr};
    };

    struct ProcessCodeLayout {
        U64 start;
        U64 offset;

        std::vector<U8> binaryimage; // Image of all binaries that will be loaded into the device's memory
    };

    enum class ProcessAddressSpace {
        AddressSpace32Bit,
        AddressSpace64BitOld,
        AddressSpace32BitNoReserved,
        AddressSpace64Bit
    };

    constexpr auto SwitchPageSize{1 << 12};

    // https://switchbrew.org/wiki/Memory_layout
    constexpr auto BaseAddr{0x80000000};
    class SlabHeap {
    public:
        static constexpr auto BaseAddress{BaseAddr + 0xE5000};
        static constexpr auto Size{0xA21000};
    };
}