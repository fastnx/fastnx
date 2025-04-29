#pragma once

#include <unordered_map>
#include <memory>
#include <common/types.h>
namespace FastNx {
    constexpr auto SwitchMemorySize{4_GBYTES};


    class MemoryBacking {
    public:
        virtual ~MemoryBacking() = default;
        virtual void Map(U8 *guest, U64 hostaddr, U64 size) = 0;
    };

    class NxAllocator final : public MemoryBacking {
    public:
        NxAllocator();
        ~NxAllocator() override;

        std::span<U8> GetSpan(U64 baseaddr = {}, U64 offset = {}, bool ishost = false) const;
        std::span<U8> InitializeGuestAs(U64 aswidth);

        void Map(U8 *guest, U64 hostaddr, U64 size) override;
    private:
        U64 sharedfd{};
        U64 size{SwitchMemorySize};
        U8 *hostptr{nullptr};
        U8 *guestptr{nullptr};


        std::unordered_map<U64, U64> alloclists;
    };

    using MemoryBackingPtr = std::shared_ptr<MemoryBacking>;
}