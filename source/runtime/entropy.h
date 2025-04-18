#pragma once

#include <common/types.h>
namespace FastNx::Runtime {
    enum class EngineType {
        Mbedtls,
        Urandom
    };
    void GetEntropy(const std::span<U8> &buffer, EngineType type = EngineType::Mbedtls);
}