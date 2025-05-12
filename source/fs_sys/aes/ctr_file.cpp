#include <boost/endian/detail/endian_reverse.hpp>
#include <boost/container/small_vector.hpp>

#include <common/container.h>
#include <fs_sys/vfs/offset_file.h>
#include <fs_sys/aes/ctr_file.h>

namespace FastNx::FsSys::Aes {
    constexpr auto CtrSectorSize{16};

    CtrFile::CtrFile(const VfsBackingFilePtr &file, const Crypto::Key128 &key, const std::array<U8, 16> &iv, const U64 starts, const U64 size) : VfsBackingFile(file->path), ctr(iv) {
        encfile = [&] -> VfsBackingFilePtr {
            if (starts && size)
                return std::make_shared<Vfs::OffsetFile>(file, file->path, starts, size);
            return file;
        }();
        ctroffset = starts / CtrSectorSize;

        decrypt.emplace(ToSpan(key), Crypto::AesMode::Decryption, Crypto::AesType::AesCtr128);
    }

    CtrFile::operator bool() const {
        return encfile != nullptr;
    }
    U64 CtrFile::GetSize() const {
        return encfile->GetSize();
    }

    void CtrFile::UpdateCtr(const U64 offset) {
        U64 sector{offset / CtrSectorSize + ctroffset};
        boost::endian::endian_reverse_inplace(sector);

        std::memcpy(&ctr[8], &sector, sizeof(sector));
        decrypt->SetupIv(ctr);
    }

    U64 CtrFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        const auto aligment{offset % CtrSectorSize};
        if (offset + size > GetSize())
            return {};

        std::scoped_lock lock{shared};
        const auto startblock{offset - aligment};

        if (!aligment) {
            UpdateCtr(offset);
            const auto result{encfile->ReadSome(std::span{dest, size}, offset)};
            if (decrypt->Process(dest, dest, result) == result)
                return result;
        }
        const auto padding{CtrSectorSize - aligment};

        boost::container::small_vector<U8, CtrSectorSize> buffer(CtrSectorSize);

        UpdateCtr(startblock); encfile->ReadSome(ToSpan(buffer), startblock);
        if (decrypt->Process(buffer.data(), buffer.data(), buffer.size())) {
            if (size + aligment < CtrSectorSize) {
                std::memcpy(dest, buffer.data() + aligment, size);
                return size;
            }
            std::memcpy(dest, buffer.data() + aligment, padding);
        }
        if (const auto count{size - padding})
            return padding + ReadSome(std::span{dest + padding, count}, offset + padding);
        return size;
    }
    U64 CtrFile::WriteTypeImpl(const U8 *source, U64 size, U64 offset) {
        std::unreachable(); std::terminate();
    }
}
