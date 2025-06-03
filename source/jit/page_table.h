#pragma once
#include <cstring>
#include <vector>
#include <common/types.h>

#include <kernel/types/kprocess.h>
namespace FastNx::Jit {
    enum class TableType : U32 {
        None,
        Code,
        Stack
    };
    class PageTable {
    public:
        explicit PageTable(const std::shared_ptr<Kernel::Types::KProcess> &process);
        void CreateTable(TableType type, void *begin, U64 size);

        TableType Contains(void *usertable, U64 size) const;
        U8 *GetTable(const void *useraddr) const;

        U64 GetPage(const void *begin) const;

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
        std::map<TableType, std::pair<U64, U64>> tableinfo;
    };
}
