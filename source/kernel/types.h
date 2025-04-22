#pragma once

#include <list>
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

    struct ProcessCodeLayout {
        U64 start;
        U64 offset;

        std::list<std::pair<U64, std::vector<U8>>> procimage; // The entire process image that will be loaded into the kernel code section
        std::list<U64> bsslayoutsize;
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