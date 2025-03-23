#include <cstring>
#include <crypto/checksum.h>

namespace FastNx::Crypto {
    Checksum::Checksum(const ChecksumType type) {
        context = new mbedtls_md_context_t;
        mbedtls_md_init(context);

        const auto *digest = [&] -> const mbedtls_md_info_t* {
            switch (type) {
                case ChecksumType::Sha256:
                    return mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
                default:
                    return nullptr;
            }
        }();
        NX_ASSERT(mbedtls_md_setup(context, digest, 0) == 0);
        info = mbedtls_md_info_from_ctx(context);
    }

    U64 Checksum::Update(const U8 *source, const U64 size) {
        if (!source || !size)
            return {};
        if (done)
            if (mbedtls_md_starts(context) == 0)
                done = {};

        if (mbedtls_md_update(context, source, size) != 0)
            return {};
        return size;
    }

    U64 Checksum::Finish(const std::span<U8> &result) {
        std::memset(result.data(), 0, result.size());
        auto mdsize{mbedtls_md_get_size(info)};
        if (result.size() >= mdsize)
            if (mbedtls_md_finish(context, result.data()) == 0)
                done = true;
        if (!done)
            mdsize = 0;
        return mdsize;
    }

    std::vector<U8> Checksum::Finish() {
        std::vector<U8> result(mbedtls_md_get_size(info));
        NX_ASSERT(Finish(std::span(result)) == result.size());
        return result;
    }

    ChecksumType Checksum::GetType() const {
        switch (mbedtls_md_get_type(info)) {
            case MBEDTLS_MD_SHA256:
                return ChecksumType::Sha256;
            default:
                return ChecksumType::None;
        }
    }

    Checksum::~Checksum() {
        if (context)
            mbedtls_md_free(context);
        delete context;
    }
}
