#include <boost/endian.hpp>
#include <common/container.h>
#include <crypto/safe_aes.h>

namespace FastNx::Crypto {
    SafeAes::SafeAes(const std::span<U8> &key, const AesMode _mode, AesType _type) {
        assert(!key.empty());

        context = new mbedtls_cipher_context_t;
        assert(mbedtls_cipher_setup(context, mbedtls_cipher_info_from_type(static_cast<mbedtls_cipher_type_t>(_type))) == 0);
        if (mbedtls_cipher_get_key_bitlen(context) != static_cast<I32>(key.size() * 8))
            return;
        assert(mbedtls_cipher_setkey(context, key.data(), key.size() * 8, _mode == AesMode::Decryption ? MBEDTLS_DECRYPT : MBEDTLS_ENCRYPT) == 0);

    }

    SafeAes::~SafeAes() {
        if (context)
            mbedtls_cipher_free(context);

        delete context;
    }

    void SafeAes::Setup(const U64 sector, const std::span<U8> &vector) {
        mbedtls_cipher_set_iv(context, vector.data(), vector.size());
        if (sector % mbedtls_cipher_get_block_size(context) == 0)
            sectorsz = sector;
    }

    U64 SafeAes::Process(U8 *dest, const U8 *source, const U64 size) {
        U8 *destination = [&] {
            if (dest != source)
                return dest;
            if (auxbuffer.size() < size)
                auxbuffer.resize(size);
            return auxbuffer.data();
        }();

        const bool isEcb{mbedtls_cipher_get_type(context) == MBEDTLS_CIPHER_AES_128_ECB};
        assert(mbedtls_cipher_reset(context) == 0);

        U64 processed{};
        for (U64 offset{}; offset <= size && isEcb; offset += 16) {
            U64 outsize{};
            mbedtls_cipher_update(context, source + offset, 16, destination + offset, &outsize);
            processed += outsize;
        }

        for (U64 offset{}; offset < size; ) {
            const U64 stride{std::min(size - processed, 2048UL)};
            U64 output{};
            mbedtls_cipher_update(context, source + offset, stride, destination + offset, &output);

            offset += output;
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

    U64 SafeAes::ProcessXts(U8 *dest, const U8 *source, const U64 size, const U64 starts) {
        U64 count{};
        tweak = starts / sectorsz;
        std::array<U8, 16> thisiv;
        for (U64 offset{}; offset < size; offset += sectorsz) {
            {
                Copy(thisiv, GetNintendoTweak(tweak));
            }
            Setup(sectorsz, thisiv);
            if (const auto result{Process(dest + offset, source + offset, sectorsz)})
                count += result;
            else break;
            tweak++;
        }
        return count;
    }
}
