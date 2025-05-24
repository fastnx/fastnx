#pragma once

#include <fs_sys/types.h>
#include <crypto/types.h>
#include <crypto/aes_cipher.h>
namespace FastNx::FsSys::Aes {
    class XtsFile final : public VfsBackingFile {
    public:
        XtsFile(const VfsBackingFilePtr &file, const Crypto::Key256 &key, U64 starts = 0, U64 size = 0);

        explicit operator bool() const override;
        U64 GetSize() const override;
        void SetSize(U64 newsize) override;

        VfsBackingFilePtr encfile{};
        std::optional<Crypto::AesCipher> encrypt{};
        std::optional<Crypto::AesCipher> decrypt{};

        bool doublebuf{};
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;
    };
}