#pragma once

#include <unordered_map>
#include <memory>
#include <boost/container_hash/hash.hpp>
#include <common/types.h>

#include <jit/page_table.h>
namespace FastNx {
    constexpr auto SwitchMemorySize{4_GBYTES};


    class MemoryBacking {
    public:
        explicit MemoryBacking(const std::shared_ptr<Jit::PageTable> &table) : pagetable(table) {}
        virtual ~MemoryBacking() = default;
        virtual void Map(U8 *guest, U64 hostaddr, U64 size) = 0;
        virtual void Reprotec(U8 *guest, U64 size, I32 protection) = 0;
        virtual bool CanAllocate(const U8 *region, U64 size) = 0;

        const std::shared_ptr<Jit::PageTable> pagetable;
    };

    using PagingType = std::pair<U8 *, U64>;
    struct PagingKey {
        auto operator()(const PagingType &key) const {
            size_t result{};
            boost::hash_combine(result, key.first);
            boost::hash_combine(result, key.second);

            return result;
        }
    };

    class SysAllocator final : public MemoryBacking {
    public:
        explicit SysAllocator(const std::shared_ptr<Jit::PageTable> &table);
        ~SysAllocator() override;

        std::span<U8> GetSpan(U64 baseaddr = {}, U64 offset = {}, bool ishost = false) const;
        std::span<U8> InitializeGuestAs(U64 aswidth, U64 assize);

        void Map(U8 *guest, U64 hostaddr, U64 size) override;
        bool CanAllocate(const U8 *region, U64 size) override;
        void Reprotec(U8 *guest, U64 size, I32 protection) override;
    private:
        U64 sharedfd{};
        U64 size{SwitchMemorySize};
        U8 *hostptr{nullptr};
        U8 *guestptr{nullptr};


        std::unordered_map<PagingType, U64, PagingKey> alloclists;
    };

    using MemoryBackingPtr = std::shared_ptr<MemoryBacking>;
}