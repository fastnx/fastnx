#pragma once
#include <cstring>
#include <vector>
#include <common/types.h>

// ReSharper disable once CppUnusedIncludeDirective
#include <kernel/types.h>
namespace FastNx::Jit {
    class PageTable {
    public:
        explicit PageTable(const std::shared_ptr<Kernel::Types::KProcess> &process);
        void CreateTable(void *begin, U64 size);

        bool Contains(void *usertable, U64 size) const;
        U8 *GetTable(const void *useraddr) const;

        U64 GetPage(const void *begin);

        template<typename T>
        T Read(const void *useraddr) {
            T type;
            std::memcpy(&type, GetTable(useraddr), sizeof(T));
            return type;
        }
        template<typename T>
        void Write(const void *useraddr, const T &value) {
            std::memcpy(GetTable(useraddr), &value, sizeof(T));
        }
        std::vector<void *> table;
    };
}
