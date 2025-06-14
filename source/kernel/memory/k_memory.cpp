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

    KMemory::KMemory(Kernel &_kernel) : table(_kernel.pagetable), kernel(_kernel), blockslist(std::make_shared<KMemoryBlockManager>(kernel)) {}

    void KMemory::InitializeForProcess(const std::shared_ptr<Types::KProcess> &process, const Svc::CreateProcessParameter &proccfg) {
        const auto width{GetWidthAs(proccfg.addrspace)};
        if (width < 39)
            return;
        processwidth = width;

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


        table->Initialize(process);
        /*
        for (const auto &[type, offset]: aslrlist) {
            if (auto *segment{GetSegment(type)}; !segment->empty())
                *segment = std::span{segment->begin().base() + offset, segment->size()};
        }
        */
    }

    void KMemory::MapSegmentMemory(const std::span<U8> &segment, const U64 begin, U64 size, const bool fill, const KMemoryBlock &block) const {
        U8* memory{segment.begin().base() + begin};
        if (!size)
            size = reinterpret_cast<U64>(segment.end().base() - begin);

        blockslist->Map({memory, block});
        if (fill)
            std::memset(memory, 0, size);
    }
    void KMemory::MapCodeMemory(const U64 begin, const U64 size, const std::vector<U8> &content) const {
        const auto _size{boost::alignment::align_up(size, SwitchPageSize)};
        NX_ASSERT(_size > content.size());

        MapSegmentMemory(code, begin, _size, false, KMemoryBlock{
            .pagescount = _size / SwitchPageSize,
            .state = MemoryState{MemoryTypeValues::Code}
        });
        U8 *memory{code.begin().base() + begin};
        std::memcpy(memory, content.data(), content.size());
        // Touching the pages adjacent to the content
        std::memset(memory + content.size(), 0, _size - content.size());

        table->DelimitTable(Jit::TableType::Code, memory, _size);
    }

    void KMemory::MapTlsMemory(const U64 begin, const U64 size) const {
        MapSegmentMemory(tlsio, begin, size, true, KMemoryBlock{
            .pagescount = size / SwitchPageSize,
            .state = MemoryState{MemoryTypeValues::ThreadLocal}
        });
    }

    void KMemory::MapStackMemory(const U64 begin, const U64 size) const {
        MapSegmentMemory(stack, begin, size, true, KMemoryBlock{
            .pagescount = size / SwitchPageSize,
            .state = MemoryState{MemoryTypeValues::Stack}
        });

        const U8 *stackit{stack.begin().base() + begin};
        table->DelimitTable(Jit::TableType::Stack, stackit, size);
    }

    void KMemory::SetMemoryPermission(const U64 begin, const U64 size, const I32 permission) const {
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

    void KMemory::FillMemory(const U64 begin, const U8 constant, const U64 size) const {
        U8 *memory{code.begin().base() + begin};

        const auto *beginpage{static_cast<U8 *>(boost::alignment::align_down(memory, SwitchPageSize))};
        const auto *endpage{static_cast<U8 *>(boost::alignment::align_up(memory + size, SwitchPageSize))};
        if (blockslist->IsMappedInRange(beginpage, endpage))
            std::memset(memory, constant, size);
    }

    std::optional<MemoryInfo> KMemory::QueryMemory(const U8 *begin) const {
        if (const auto block{blockslist->FindBlock(begin)})
            return MemoryInfo{
                .base = block->first,
                .size = block->second->pagescount * SwitchPageSize,
                .type = block->second->state._type,
                .permission = block->second->permission,
            };
        if (begin < addrspace.begin().base())
            return MemoryInfo{
                .base = addrspace.end().base(),
                .size = 0 - reinterpret_cast<U64>(addrspace.end().base()),
                .type = MemoryTypeValues::Inaccessible,
            };
        return std::nullopt;
    }
}
