#pragma once

#include <mbedtls/cipher.h>
namespace FastNx::Crypto {
    enum class AesMode {
        None,
        Encryption,
        Decryption
    };
    enum class AesType {
        AesXts128 = MBEDTLS_CIPHER_AES_128_XTS,
        AesCtr128 = MBEDTLS_CIPHER_AES_128_CTR,
        AesEcb128 = MBEDTLS_CIPHER_AES_128_ECB,
    };
    class SafeAes {
    public:
        explicit SafeAes(const std::span<const U8> &key, AesMode _mode, AesType _type);
        ~SafeAes();

        void Setup(U64 sector, const std::span<U8> &vector);
        U64 Process(U8 *dest, const U8 *source, U64 size);
        U64 ProcessXts(U8 *dest, const U8 *source, U64 size, U64 starts = 0);

        std::vector<U8> auxbuffer;
    private:
        U64 sectorsz{0x200};

        mbedtls_cipher_context_t *context;
        U64 tweak{};
    };
}
