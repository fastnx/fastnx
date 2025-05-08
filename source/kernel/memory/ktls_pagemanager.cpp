#include <kernel/memory/ktls_pagemanager.h>

namespace FastNx::Kernel::Memory {
    KTlsPageManager::KTlsPageManager(U8 *tlsr) : tlsbase(tlsr) {
        freeslots.resize(TlsSlotCount);
        NX_ASSERT(freeslots.size() == TlsSlotCount);


        std::ranges::fill(freeslots, 1);
    }

    U8 * KTlsPageManager::Allocate() {
        U8 *begin{tlsbase};
        for (auto &tlspage: freeslots) {
            [[likely]] if (tlspage) {
                tlspage = {};
                break;
            }
            begin += TlsEntrySize;
        }
        if (begin == tlsbase + freeslots.size() * TlsEntrySize)
            return nullptr;
        return begin;
    }

    void KTlsPageManager::Free(const U8 *tls) {
        freeslots[(tlsbase - tls) / TlsEntrySize] = true;
    }

    bool KTlsPageManager::Allocatable() const {
        for (const auto free: freeslots)
            if (free)
                return true;
        return {};
    }
}
