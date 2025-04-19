#include <boost/align/align_up.hpp>
#include <boost/align/align_down.hpp>
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
        const auto blockoffset{boost::alignment::align_down(offset, 16)};
        if (blockoffset + size > GetSize())
            return {};
        UpdateCtr(blockoffset);

        const auto rbcount{encfile->ReadSome(std::span{dest, size}, blockoffset)};
        if (rbcount != size)
            return {};
        decrypt->Process(dest, dest, rbcount);
        if (auto aligment{offset % CtrSectorSize}) {
            if (static_cast<ssize_t>(size - aligment) > 0)
                std::memmove(dest, dest + aligment, size - aligment);

            const auto slice{size % CtrSectorSize};
            if (slice + aligment > CtrSectorSize)
                aligment = CtrSectorSize - slice;
            boost::container::small_vector<U8, CtrSectorSize> buffer(aligment + slice);

            if (size < CtrSectorSize - aligment) {
                if (ReadSome(ToSpan(buffer), blockoffset))
                    std::memcpy(dest, buffer.data() + aligment, size);
            } else if (ReadSome(ToSpan(buffer), blockoffset + size - slice))
                std::memcpy(dest + size - aligment, buffer.data() + slice, aligment);
        }

        return size;
    }
    U64 CtrFile::WriteTypeImpl(const U8 *source, U64 size, U64 offset) {
        std::unreachable(); std::terminate();
    }
}
