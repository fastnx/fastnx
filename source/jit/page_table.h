#pragma once
#include <cstring>
#include <vector>
#include <common/types.h>
#include <kernel/memory/k_memory.h>

namespace FastNx::Jit {
    class PageTable {
    public:
        explicit PageTable(const std::shared_ptr<Kernel::Memory::KMemory> &memory);
        void CreateTable(void *begin, U64 size);

        U8 *GetTable(const void *useraddr) const;

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
