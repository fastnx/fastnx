#include <fs_sys/vfs/offset_file.h>

namespace FastNx::FsSys::Vfs {
    OffsetFile::OffsetFile(const VfsBackingFilePtr &backing, const FsPath &_path, const U64 offset, const U64 size, const bool isHuge) : VfsBackingFile(_path), size(size), begin(offset), filesrc(backing) {
        eof = offset + size;
        if (isHuge)
            hugefs = std::dynamic_pointer_cast<ReFs::HugeFile>(backing);
        NX_ASSERT(*this);
    }

    OffsetFile::operator bool() const {
        return filesrc->GetSize() >= begin + size;
    }

    U64 OffsetFile::GetSize() const {
        if (filesrc)
            return std::min(filesrc->GetSize(), eof - begin);
        return size;
    }
    void OffsetFile::SetSize(U64 newsize) {
        std::terminate();
    }
    U64 OffsetFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        if (begin + offset + size > eof)
            return {};

        const auto result = [&] {
            if (hugefs)
                return hugefs->ReadTypeFaster(dest, size, begin + offset);
            return filesrc->ReadType(dest, size, begin + offset);
        }();
        return result;
    }

    U64 OffsetFile::WriteTypeImpl(const U8 *source, U64 size, U64 offset) {
        std::unreachable(); std::terminate();
    }
}
