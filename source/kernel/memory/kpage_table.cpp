#include <boost/align/detail/align_down.hpp>
#include <boost/align/detail/align_up.hpp>
#include <kernel/memory/kpage_table.h>

#include <fs_sys/npdm.h>

namespace FastNx::Kernel::Memory {
    namespace AddressSpace64Bits {
        constexpr auto AliasSize{64_GBYTES};
        constexpr auto HeapSize{8_GBYTES};
        constexpr auto StackSize{2_GBYTES};

    }
    constexpr auto RegionAlignment{0x200000};

    void KPageTable::CreateForProcess([[maybe_unused]] const std::shared_ptr<Types::KProcess> &process, const Svc::CreateProcessParameter &proccurr, [[maybe_unused]] const ProcessCodeLayout &codeset) {
        const auto spacewidth = [&] {
            switch (proccurr.addrspace) {
                case ProcessAddressSpace::AddressSpace64Bit:
                    return 39;
                default:
                    std::unreachable();
            }
        }();
        NX_ASSERT(baseas != nullptr);
        addrspace = {baseas, baseas + (1ULL << 39)};

        switch (spacewidth) {
            case 39: {
                auto *start{static_cast<U8 *>(boost::alignment::align_down(baseas, RegionAlignment))};
                const auto size{static_cast<U8 *>(boost::alignment::align_up(baseas + sizeforeom, RegionAlignment)) - start};
                coderegion = {start, static_cast<U64>(size)};
            }
            default: {}
        }
        // Total size used by all regions
        const auto totalsize{coderegion.size()};
        // Calculates remaining free space in the address space
        const auto [available, aslrstart] = [&] {
            if (coderegion.begin() - aslrregion.begin() >= addrspace.end() - coderegion.end())
                return std::make_pair(coderegion.begin() - aslrregion.begin(), aslrregion.begin().base());
            return std::make_pair(addrspace.end() - coderegion.end(), coderegion.end().base());
        }();
        NX_ASSERT(totalsize < static_cast<U64>(available));
        // Final offset of ASLR
        const auto maxaslr{available - totalsize};

        if (maxaslr && proccurr.enbaslr) {}
    }
}
