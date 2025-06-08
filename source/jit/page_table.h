#pragma once
#include <cstring>
#include <vector>
#include <common/types.h>
#include <common/allocators.h>

#include <kernel/types.h>


namespace FastNx::Jit {
    enum class TableType : U32 {
        Allocated,
        Code,
        Stack
    };

    enum class PageAttributeType {
        Unmapped,
        Mapped,
    };

    class PageTable {
    public:
        static constexpr auto PageAttrBitsCount{2};
        struct Page {
            auto GetPageAttr() const {
                // ReSharper disable once CppRedundantParentheses
                return static_cast<PageAttributeType>(taggedptr & ((1 << PageAttrBitsCount) - 1));
            }
            void SetPageAttr(PageAttributeType type) {
                taggedptr |= static_cast<U64>(type);
            }

            auto operator ()(const U64 offset) const -> void* {
                auto result{taggedptr};
                if (!result)
                    return {};
                result &= ~0 << PageAttrBitsCount;
                return reinterpret_cast<void *>(result + offset);
            }

            Page() = default;
            explicit Page(const void *value, const PageAttributeType type = PageAttributeType::Mapped) :
                taggedptr(reinterpret_cast<U64>(value)) {
                SetPageAttr(type);
            }
            U64 taggedptr{};
        };
        static_assert(sizeof(Page) == sizeof(void *));

        PageTable() = default;
        void Initialize(const std::shared_ptr<Kernel::Types::KProcess> &process);
        void CreateTable(void *begin, void *host, U64 offset, U64 size);
        void DelimitTable(TableType _type, const void *begin, U64 size);

        std::pair<PageAttributeType, TableType> Contains(void *usertable, U64 size);
        U8 * GetTable(const void *useraddr);

        template<typename T>
        T Read(const void *useraddr) {
            T type;
            const auto *pointer{GetTable(useraddr)};
            if (!pointer)
                throw std::bad_alloc{};
            std::memcpy(&type, pointer, sizeof(T));
            return type;
        }
        template<typename T>
        void Write(const void *useraddr, const T &value) {
            auto *pointer{GetTable(useraddr)};
            if (!pointer)
                throw std::bad_alloc{};
            std::memcpy(pointer, &value, sizeof(T));
        }
        SwappableVector<Page> table; // Nossa table de endereços
        std::vector<Page> backing; // Memória fisica

        U64 pagetablewidth{};

        std::vector<std::pair<TableType, std::pair<void *, void*>>> tableinfo; // Demarca o dominio das paginas
    };
}
