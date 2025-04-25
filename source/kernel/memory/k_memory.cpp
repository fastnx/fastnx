#include <chrono>
#include <random>

#include <boost/align/align_up.hpp>

#include <kernel/kernel.h>
#include <kernel/memory/k_memory.h>
#include <kernel/types.h>



namespace FastNx::Kernel::Memory {
    constexpr auto RegionAlignment{0x200000};

    namespace AddressSpace64Bits {
        constexpr auto AliasSize{64_GBYTES};
        constexpr auto HeapSize{8_GBYTES};
        constexpr auto StackSize{2_GBYTES};
        constexpr auto TlsIoSize{64_GBYTES};

    }

    auto GetWidthAs(const ProcessAddressSpace addrspace) -> U32 {
        if (addrspace == ProcessAddressSpace::AddressSpace32Bit)
            return 36;
        if (addrspace == ProcessAddressSpace::AddressSpace64Bit)
            return 39;
        return {};
    }
    auto GetRandom(const I32 maxval) {
        const auto seed{std::chrono::system_clock::now().time_since_epoch().count()};
        static std::mt19937 generator(seed);

        std::uniform_int_distribution distribution(0, maxval);
        return distribution(generator);
    }

    KMemory::KMemory(Kernel &_kernel) : kernel(_kernel) {}

    void KMemory::InitializeProcessMemory(const Svc::CreateProcessParameter &proccfg) {
        const auto width{GetWidthAs(proccfg.addrspace)};
        if (width < 39)
            return;
        addrspace = kernel.host->InitializeGuestAs(1ULL << width);

        switch (width) {
            case 39:
                code = std::span{addrspace.begin().base(), boost::alignment::align_up(addrspace.size(), RegionAlignment)};
                alias = std::span{code.end().base(), AddressSpace64Bits::AliasSize};
                heap = std::span{alias.end().base(), AddressSpace64Bits::HeapSize};
                stack = std::span{heap.end().base(), AddressSpace64Bits::StackSize};
                tlsio = std::span{stack.end().base(), AddressSpace64Bits::TlsIoSize};

                break;
            default:
                std::unreachable();
        }

        std::vector<std::pair<SegmentType, U64>> aslrlist;
        for (const auto _type: EnumRange(SegmentType::Code, SegmentType::TlsIo))
            aslrlist.emplace_back(_type, GetRandom(0x6400) * RegionAlignment);

        for (const auto &[type, offset]: aslrlist) {
            if (auto *segment{GetSegment(type)}; !segment->empty())
                *segment = std::span{segment->begin().base() + offset, segment->size()};
        }

        ptblocks.Initialize(addrspace);
    }



}
