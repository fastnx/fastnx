#include <common/container.h>
#include <fs_sys/xts_file.h>

namespace FastNx::FsSys {
    XtsFile::XtsFile(const VfsBackingFilePtr &file, const Crypto::SourceKey<256> &key) : VfsBackingFile(file->path), encfile(file) {
        std::vector<U8> _tweakbytes(sizeof(key));
        Copy(_tweakbytes, key);

        using Mode = Crypto::AesMode;
        constexpr auto AesType{Crypto::AesType::AesXts128};

        if (file->mode != FileModeType::ReadOnly)
            encrypt.emplace(std::span(_tweakbytes), Mode::Encryption, AesType);
        decrypt.emplace(std::span(_tweakbytes), Mode::Decryption, AesType);
    }

    XtsFile::operator bool() const {
        return encfile != nullptr;
    }

    U64 XtsFile::GetSize() const {
        return encfile->GetSize();
    }

    U64 XtsFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        std::optional<std::vector<U8>> ebuffer;
        auto output = [&] -> std::span<U8> {
            if (!doublebuf)
                return {dest, encfile->ReadType(dest, size, offset)};
            ebuffer.emplace(encfile->ReadSome(size, offset));
            return *ebuffer;
        }();
        if (!output.size())
            return {};

        if (auto *_bytes{output.data()})
            decrypt->ProcessXts(_bytes, _bytes, size);
        return size;
    }

    U64 XtsFile::WriteTypeImpl(const U8 *source, U64 size, U64 offset) {
        std::unreachable(); std::terminate();
    }
}
