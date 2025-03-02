#include <cassert>
#include <cstring>
#include <crypto/hashsum.h>

namespace FastNx::Crypto {
    HashSum::HashSum(const HashMode mode) {
        context = new mbedtls_md_context_t;
        mbedtls_md_init(context);

        const auto* digest = [&] -> const mbedtls_md_info_t* {
            switch (mode) {
                case HashMode::Sha256:
                    return mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
                default:
                    return nullptr;
            }
        }();
        assert(mbedtls_md_setup(context, digest, 0) == 0);
        info = mbedtls_md_info_from_ctx(context);
    }

    U64 HashSum::Update(const U8 *source, const U64 size) {
        if (!source || !size)
            return {};
        if (done)
            if (mbedtls_md_starts(context) == 0)
                done = {};

        mbedtls_md_update(context, source, size);
        return size;
    }

    U64 HashSum::Finish(const std::span<U8> &result) {
        std::memset(result.data(), 0, result.size());
        auto mdsize{mbedtls_md_get_size(info)};
        if (result.size() >= mdsize)
            if (mbedtls_md_finish(context, result.data()) == 0)
                done = true;
        if (!done)
            mdsize = 0;
        return mdsize;
    }

    std::vector<U8> HashSum::Finish() {
        std::vector<U8> result(mbedtls_md_get_size(info));
        assert(Finish(std::span(result)) == result.size());
        return result;
    }

    HashMode HashSum::GetMode() const {
        switch (mbedtls_md_get_type(info)) {
            case MBEDTLS_MD_SHA256:
                return HashMode::Sha256;
            default:
                return HashMode::None;
        }
    }

    HashSum::~HashSum() {
        if (context)
            mbedtls_md_free(context);
        delete context;
    }
}
