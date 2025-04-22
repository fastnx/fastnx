#pragma once

#include <kernel/types.h>
#include <common/container.h>
#include <common/types.h>

namespace FastNx::Runtime {
    enum class EngineType {
        Mbedtls,
        Urandom
    };
    void GetEntropy(const std::span<U8> &buffer, EngineType type = EngineType::Mbedtls);

    inline void GetEntropy(Kernel::ProcessEntropy &processent) {
        GetEntropy(std::span{reinterpret_cast<U8 *>(processent.data()), ToSpan(processent).size_bytes()});
    }
}
