#pragma once

namespace FastNx::Kernel {
    class KAutoObject;
    class Kernel;

    namespace Types {
        class KThread;
        class KProcess;
    }

    constexpr auto InitialProcessId{0x51};
    constexpr auto MaximumProcessIds{300};

    struct MemoryDescriptor {
        void *begin{nullptr};
        void *end{nullptr};
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