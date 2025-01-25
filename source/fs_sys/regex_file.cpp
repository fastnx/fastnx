#include <cassert>

#include <boost/regex.hpp>
#include <fs_sys/types.h>
namespace FastNx::FsSys {
    RegexFile::RegexFile(const FsPath &_path, const std::string &pattern): VfsBackingFile(_path) {
        assert(!pattern.empty());
        try {
            const boost::regex compile{pattern};
        } catch ([[maybe_unused]] const boost::regex_error &except) {}
    }
}
