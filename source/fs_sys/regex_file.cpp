#include <cassert>
#include <ranges>
#include <algorithm>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/regex.hpp>

#include <common/container.h>
#include <common/exception.h>
#include <fs_sys/regex_file.h>
namespace FastNx::FsSys {
    RegexFile::RegexFile(const VfsBackingFilePtr &_file, const std::string &pattern): VfsBackingFile(_file->path), file(_file) {
        assert(file && !pattern.empty());
        try {
            const boost::regex compile{pattern};
            auto lines{file->GetAllLines()};
            matches.reserve(lines.size());

            for (auto forline{lines.begin()}; forline != lines.end(); ++forline) {
                if (!regex_match(*forline, compile))
                    continue;
                matches.emplace_back(std::move(*forline));
                forline = lines.erase(forline);
            }

            if (!lines.empty())
                fmt::print("Deleted lines: {}", fmt::join(lines, ", "));
        } catch (const boost::regex_error &except) {
            throw exception{"The regex due to: {}", except.what()};
        }
    }

    RegexFile::operator bool() const {
        return !matches.empty();
    }

    U64 RegexFile::GetSize() const {
        return file->GetSize();
    }
    U64 RegexFile::ReadTypeImpl(U8 *dest, const U64 size, U64 offset) {
        auto line{matches.begin()};
        U64 lastoff{};
        for (; !line->empty() && offset; ++line) {
            lastoff = offset;
            offset -= std::min(offset, line->size());
        }

        const auto _size{line->size() - lastoff};
        if (_size)
            std::memcpy(dest, line->begin().base() + lastoff, _size);
        return _size;
    }
}
