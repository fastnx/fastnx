#pragma once
#include <cstring>
#include <vector>

#include <boost/align/align_down.hpp>

#include <common/types.h>
#include <kernel/memory/k_memory.h>
#include <kernel/types.h>


namespace FastNx::Jit {
    class PageTable {
    public:
        explicit PageTable(const std::shared_ptr<Kernel::Memory::KMemory> &memory);
        void CreateTable(void *begin, U64 size);

        template<typename T>
        T Read(const void *useraddr) {
            T type;
            const auto *aligned{boost::alignment::align_down(const_cast<void *>(useraddr), Kernel::SwitchPageSize)};
            const auto offset{reinterpret_cast<U64>(useraddr) - reinterpret_cast<U64>(aligned)};

            std::memcpy(&type, static_cast<U8 *>(table[reinterpret_cast<U64>(aligned)]) + offset, sizeof(T));
            return type;
        }
        std::vector<void *> table;
    };
}
