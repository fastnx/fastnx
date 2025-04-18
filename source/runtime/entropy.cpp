#include <mbedtls/entropy.h>

#include <fs_sys/refs/buffered_file.h>
#include <common/container.h>
#include <runtime/entropy.h>

void FastNx::Runtime::GetEntropy(const std::span<U8> &buffer, const EngineType type) {
    mbedtls_entropy_context entropy;
    mbedtls_entropy_init(&entropy);
    if (type == EngineType::Mbedtls) {
        mbedtls_entropy_func(&entropy, buffer.data(), buffer.size());
    } else if (type == EngineType::Urandom) {
        if (FsSys::ReFs::BufferedFile unrandom{"/dev/urandom"})
            unrandom.ReadSome(ToSpan(buffer));
    }
    mbedtls_entropy_free(&entropy);

}
