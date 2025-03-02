#pragma once

#include <vector>
#include <mbedtls/md.h>

#include <common/types.h>
namespace FastNx::Crypto {
    enum class HashMode {
        None,
        Sha256
    };
    class HashSum {
    public:
        explicit HashSum(HashMode mode = HashMode::Sha256);

        U64 Update(const U8 *source, U64 size);
        U64 Finish(const std::span<U8> &result);
        std::vector<U8> Finish();

        template<typename T>
        auto Update(const std::vector<T> &content) {
            return Update(reinterpret_cast<const U8 *>(content.data()), content.size() * sizeof(T));
        }
        HashMode GetMode() const;
        ~HashSum();

    private:
        bool done{true};
        mbedtls_md_context_t *context{nullptr};
        const mbedtls_md_info_t *info{nullptr};
    };
}