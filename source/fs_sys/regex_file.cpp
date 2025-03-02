#include <cassert>

#include <boost/regex.hpp>
#include <fs_sys/refs/buffered_file.h>
#include <fs_sys/regex_file.h>
namespace FastNx::FsSys {
    RegexFile::RegexFile(const FsPath &_path, const std::string &pattern): VfsBackingFile(_path) {
        assert(exists(_path) && !pattern.empty());
        file = std::make_shared<ReFs::BufferedFile>(_path);
        if (!file)
            return;
        try {
            const boost::regex compile{pattern};
        } catch ([[maybe_unused]] const boost::regex_error &except) {
        }
    }

    RegexFile::operator bool() const {
        return true;
    }

    U64 RegexFile::GetSize() const {
        return file->GetSize();
    }
    U64 RegexFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        return file->ReadType(dest, size, offset);
    }
}
