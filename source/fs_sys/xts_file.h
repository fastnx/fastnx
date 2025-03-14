#pragma once

#include <fs_sys/types.h>
#include <crypto/types.h>
#include <crypto/safe_aes.h>
namespace FastNx::FsSys {
    class XtsFile final : public VfsBackingFile {
    public:
        XtsFile(const VfsBackingFilePtr &file, const Crypto::SourceKey<256> &key);

        explicit operator bool() const override;
        U64 GetSize() const override;

        VfsBackingFilePtr encfile{};
        std::optional<Crypto::SafeAes> encrypt{};
        std::optional<Crypto::SafeAes> decrypt{};

        bool doublebuf{};
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;
    };
}