#pragma once

#include <mutex>
#include <fs_sys/types.h>
#include <crypto/types.h>
#include <crypto/aes_cipher.h>

namespace FastNx::FsSys::Aes {
    class CtrFile final : public VfsBackingFile {
    public:
        CtrFile(const VfsBackingFilePtr &file, const Crypto::Key128 &key, const std::array<U8, 16> &iv, U64 starts = 0, U64 size = 0);

        explicit operator bool() const override;
        U64 GetSize() const override;
        void SetSize(U64 newsize) override;

        VfsBackingFilePtr encfile{};
        std::optional<Crypto::AesCipher> decrypt{};
        std::array<U8, 16> ctr;

        U64 ctroffset{};
        std::recursive_mutex shared;

    private:
        void UpdateCtr(U64 offset);

        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;
    };
}