#include <chrono>
#include <cstring>
#include <random>

#include <boost/align/align_up.hpp>
#include <boost/align/align_down.hpp>

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

        constexpr auto TotalSize{AliasSize + HeapSize + StackSize + TlsIoSize};

    }

    auto GetWidthAs(const ProcessAddressSpace addrspace) -> U64 {
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
        if (!blockslist)
            blockslist.emplace(kernel.poffset);
        const auto codesize{boost::alignment::align_up(proccfg.codenumpages * SwitchPageSize, RegionAlignment)};

        switch (width) {
            case 39:
                addrspace = blockslist->Initialize(1ULL << width, AddressSpace64Bits::TotalSize + codesize, kernel);
                code = std::span{addrspace.begin().base(), codesize};
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

        /*
        for (const auto &[type, offset]: aslrlist) {
            if (auto *segment{GetSegment(type)}; !segment->empty())
                *segment = std::span{segment->begin().base() + offset, segment->size()};
        }
        */
    }

    void KMemory::MapCodeMemory(const U64 begin, const U64 size, const std::vector<U8> &content) {
        U8 *memory{code.begin().base() + begin};
        NX_ASSERT(size > content.size());
        const auto _size{boost::alignment::align_up(size, SwitchPageSize)};

        blockslist->Map({memory, KMemoryBlock{
            .pagescount = _size / SwitchPageSize,
            .state = MemoryState{MemoryTypeValues::Code}
        }});
        std::memcpy(memory, content.data(), content.size());
    }

    void KMemory::MapTlsMemory(const U64 begin, U64 size) {
        U8 *memory{tlsio.begin().base() + begin};
        if (!size)
            size = reinterpret_cast<U64>(tlsio.end().base() - begin);
        blockslist->Map({memory, KMemoryBlock{
            .pagescount = size / SwitchPageSize,
            .state = MemoryState{MemoryTypeValues::ThreadLocal}
        }});

        std::memset(memory, 0, size);
    }

    void KMemory::SetMemoryPermission(const U64 begin, const U64 size, const I32 permission) {
        U8 *memory{code.begin().base() + begin};
        const auto _size{boost::alignment::align_up(size, SwitchPageSize)};

        U64 reprotected{};
        auto callback = [&](const KMemoryBlock &block) {
            if (block.state == MemoryState{MemoryTypeValues::Code}) {
                KMemoryBlock updateinfo{
                    .pagescount = _size / SwitchPageSize,
                    .permission = static_cast<U8>(permission),
                };
                if (block.permission != permission)
                    blockslist->Reprotect({memory + reprotected, updateinfo});
            }
            reprotected += block.pagescount * SwitchPageSize;
        };

        blockslist->ForEach({memory, KMemoryBlock{
            .pagescount = _size / SwitchPageSize,
        }}, callback);
    }

    void KMemory::FillMemory(const U64 begin, const U8 constant, const U64 size) {
        U8 *memory{code.begin().base() + begin};

        const auto *beginpage{static_cast<U8 *>(boost::alignment::align_down(memory, SwitchPageSize))};
        const auto *endpage{static_cast<U8 *>(boost::alignment::align_up(memory + size, SwitchPageSize))};
        if (blockslist->IsMappedInRange(beginpage, endpage))
            std::memset(memory, constant, size);
    }
}
