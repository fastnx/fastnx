#include <boost/endian.hpp>
#include <common/container.h>
#include <crypto/aes_cipher.h>

namespace FastNx::Crypto {
    AesCipher::AesCipher(const std::span<const U8> &key, const AesMode _mode, AesType _type) {
        NX_ASSERT(!key.empty());

        context = new mbedtls_cipher_context_t;
        NX_ASSERT(mbedtls_cipher_setup(context, mbedtls_cipher_info_from_type(static_cast<mbedtls_cipher_type_t>(_type))) == 0);
        if (mbedtls_cipher_get_key_bitlen(context) != static_cast<I32>(key.size() * 8))
            return;
        NX_ASSERT(mbedtls_cipher_setkey(context, key.data(), key.size() * 8, _mode == AesMode::Decryption ? MBEDTLS_DECRYPT : MBEDTLS_ENCRYPT) == 0);

    }

    AesCipher::~AesCipher() {
        if (context)
            mbedtls_cipher_free(context);

        delete context;
    }

    void AesCipher::SetupIv(const std::span<U8> &vector, const U64 sector) {
        mbedtls_cipher_set_iv(context, vector.data(), vector.size());
        if (sector && sector % mbedtls_cipher_get_block_size(context) == 0)
            sectorsz = sector;
    }

    U64 AesCipher::Process(U8 *dest, const U8 *source, const U64 size) {
        U8 *destination = [&] {
            if (dest != source)
                return dest;
            if (auxbuffer.size() < size)
                auxbuffer.resize(size);
            return auxbuffer.data();
        }();

        NX_ASSERT(mbedtls_cipher_reset(context) == 0);

        U64 processed{}, throughput{};
        if (mbedtls_cipher_get_type(context) == MBEDTLS_CIPHER_AES_128_ECB) {
            for (; processed < size; processed += throughput)
                mbedtls_cipher_update(context, source + processed, 16, destination + processed, &throughput);
            processed += size;
        }

        for (U64 output{}; processed < size; ) {
            const U64 stride{std::min(size - processed, 2048UL)};
            mbedtls_cipher_update(context, source + processed, stride, destination + processed, &output);
            processed += output;
        }

        if (destination != dest)
            std::memcpy(dest, destination, size);
        return processed;
    }

    // https://gist.github.com/SciresM/fe8a631d13c069bd66e9c656ab5b3f7f
    auto GetNintendoTweak(U64 tweak) {
        std::array<U8, 16> iv;
        std::memset(iv.data(), 0, iv.size());
        tweak = boost::endian::native_to_big(tweak);
        std::memcpy(iv.data() + 8, &tweak, sizeof(tweak));
        return iv;
    }

    U64 AesCipher::ProcessXts(U8 *dest, const U8 *source, const U64 size, const U64 starts) {
        U64 count{};
        tweak = starts / sectorsz;
        SetupIv({}, tweak);
        std::array<U8, 16> thisiv;
        for (U64 offset{}; offset < size; offset += sectorsz) {
            {
                Copy(thisiv, GetNintendoTweak(tweak));
            }
            SetupIv(thisiv);
            if (const auto result{Process(dest + offset, source + offset, sectorsz)})
                count += result;
            else break;
            tweak++;
        }
        return count;
    }
}
