#include <chrono>
#include <cstring>
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
        ptblocks.Initialize(addrspace, 1ULL << width, kernel);

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
    }

    void KMemory::MapCodeMemory(const U64 begin, const std::vector<U8> &content) {
        U8 *memory{code.begin().base() + begin};
        const auto size{boost::alignment::align_up(content.size(), SwitchPageSize)};

        ptblocks.Map({memory, KMemoryBlock{
            .pagescount = size / SwitchPageSize,
            .state = MemoryTypeValues::Code
        }});
        std::memcpy(memory, content.data(), content.size());
    }

    void KMemory::SetMemoryPermission(const U64 begin, const U64 _size, const U32 permission) {
        U8 *memory{code.begin().base() + begin};
        const auto size{boost::alignment::align_up(_size, SwitchPageSize)};

        auto callback = [&](KMemoryBlock &block) {
            if (block.state.code)
                block.permission = permission;
        };

        ptblocks.ForEach({memory, KMemoryBlock{
            .pagescount = size / SwitchPageSize,
        }}, callback);
    }

    void KMemory::FillMemory(const U64 begin, const U8 constant, const U64 size) {
        U8 *memory{code.begin().base() + begin};

        if (ptblocks.IsMappedInRange(memory, memory + size)) {
            std::memset(memory, constant, size);
        }
    }
}
