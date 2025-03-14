#include <fs_sys/standard_file.h>

namespace FastNx::FsSys {
    StandardFile::StandardFile(std::ostream &_stream) : VfsBackingFile("vfs", FileModeType::WriteOnly), outstr(_stream) {}

    StandardFile::operator bool() const {
        return static_cast<bool>(outstr);
    }
    U64 StandardFile::GetSize() const {
        return _size;
    }
    U64 StandardFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        std::unreachable(); std::terminate();
    }
    U64 StandardFile::WriteTypeImpl(const U8 *source, const U64 size, const U64 offset) {
        outstr << std::string_view(reinterpret_cast<const char*>(source), size);
        return size;
    }
}