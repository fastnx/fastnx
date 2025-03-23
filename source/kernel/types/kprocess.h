#pragma once

#include <common/types.h>
namespace FastNx::Kernel::Types {
    class KProcess {
    public:
        KProcess() = default;

        std::array<U64, 4> entropy;
    };
}