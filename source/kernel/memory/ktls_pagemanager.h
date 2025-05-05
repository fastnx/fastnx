#pragma once
#include <boost/container/small_vector.hpp>

#include <kernel/types.h>

namespace FastNx::Kernel::Memory {
    class KTlsPageManager {
    public:
        static constexpr auto TlsEntrySize{0x200};
        static constexpr auto TlsSlotCount{SwitchPageSize / TlsEntrySize};

        explicit KTlsPageManager(U8 *tlsr);
        U8 *Allocate();
        void Free(const U8 *tls);
        bool Allocatable() const;

        U8 *tlsbase{};
    private:

        boost::container::small_vector<U8, TlsSlotCount> freeslots{TlsSlotCount};
    };
}
