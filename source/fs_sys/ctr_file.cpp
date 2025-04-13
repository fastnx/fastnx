#include <boost/align/align_up.hpp>
#include <boost/align/align_down.hpp>
#include <boost/endian/detail/endian_reverse.hpp>

#include <common/container.h>
#include <fs_sys/offset_file.h>
#include <fs_sys/ctr_file.h>

namespace FastNx::FsSys {
    constexpr auto CtrSectorSize{16};

    CtrFile::CtrFile(const VfsBackingFilePtr &file, const Crypto::Key128 &key, const std::array<U8, 16> &iv, const U64 starts, const U64 size) : VfsBackingFile(file->path), ctr(iv) {
        encfile = [&] -> VfsBackingFilePtr {
            if (starts && size)
                return std::make_shared<OffsetFile>(file, file->path, starts, size);
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
        if (offset + size > GetSize())
            return {};
        const auto blockoffset{boost::alignment::align_down(offset, 16)};
        UpdateCtr(blockoffset);

        const auto count{encfile->ReadSome(std::span{dest, size}, blockoffset)};
        if (count != size)
            return {};
        decrypt->Process(dest, dest, count);
        if (const auto aligment{offset % CtrSectorSize}) {
            std::array<U8, CtrSectorSize> buffer;
            std::memmove(dest, dest + aligment, size - aligment);

            if (const auto slice{buffer.size() - aligment}; size < slice) {
                if (ReadSome(ToSpan(buffer), blockoffset))
                    std::memcpy(dest, buffer.data() + aligment, size);
            } else if (ReadSome(ToSpan(buffer), boost::alignment::align_down(blockoffset + size, 16)))
                std::memcpy(dest + size - aligment, buffer.data() + slice, aligment);
        }

        return size;
    }
    U64 CtrFile::WriteTypeImpl(const U8 *source, U64 size, U64 offset) {
        std::unreachable(); std::terminate();
    }
}
