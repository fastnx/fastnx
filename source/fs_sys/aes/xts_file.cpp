#include <common/container.h>

#include <fs_sys/vfs/offset_file.h>

#include <fs_sys/aes/xts_file.h>

namespace FastNx::FsSys::Aes {
    XtsFile::XtsFile(const VfsBackingFilePtr &file, const Crypto::Key256 &key, const U64 starts, const U64 size) : VfsBackingFile(file->path) {
        encfile = [&] -> VfsBackingFilePtr {
            if (starts && size)
                return std::make_shared<Vfs::OffsetFile>(file, file->path, starts, size);
            return file;
        }();
        std::vector<U8> tweakbytes(sizeof(key));
        Copy(tweakbytes, key);

        using Mode = Crypto::AesMode;
        constexpr auto AesType{Crypto::AesType::AesXts128};

        if (file->mode != FileModeType::ReadOnly)
            encrypt.emplace(ToSpan(tweakbytes), Mode::Encryption, AesType);
        decrypt.emplace(ToSpan(tweakbytes), Mode::Decryption, AesType);
    }

    XtsFile::operator bool() const {
        return encfile != nullptr;
    }

    U64 XtsFile::GetSize() const {
        return encfile->GetSize();
    }

    U64 XtsFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        std::optional<std::vector<U8>> ebuffer;
        const auto output = [&] -> std::span<U8> {
            if (!doublebuf)
                return {dest, encfile->ReadType(dest, size, offset)};
            ebuffer.emplace(encfile->ReadSome(size, offset));
            return *ebuffer;
        }();
        if (!output.size())
            return {};

        if (const auto *_bytes{output.data()})
            decrypt->ProcessXts(dest, _bytes, size, offset);
        return size;
    }

    U64 XtsFile::WriteTypeImpl(const U8 *source, U64 size, U64 offset) {
        std::unreachable(); std::terminate();
    }
}
