#include <common/traits.h>
#include <common/container.h>

#include <fmt/format.h>
#include <fmt/ranges.h>
#include <boost/algorithm/hex.hpp>

#include <crypto/types.h>
#include <crypto/hashsum.h>


namespace FastNx::Crypto {
    template<U64 Size, typename T> requires (IsStringType<T>)
    constexpr std::array<U8, Size> ToArrayOfBytes(const T &string) {
        std::array<U8, Size> result{};
        if (string.size() / 2 != result.size())
            throw std::bad_cast{};

        thread_local std::vector<U8> bytes;
        if (bytes.size() < result.size())
            bytes.reserve(result.size());
        else
            bytes.clear();

        boost::algorithm::unhex(string, std::back_inserter(bytes));
        assert(Copy(result, bytes) == bytes.size());
        return result;
    }

    bool CheckNcaIntegrity(const FsSys::VfsBackingFilePtr &file) {
        const auto &fullpath{GetPathStr(file)};
        std::vector<U8> buffer(4_MBYTES);

        if (!isalnum(*fullpath.begin()))
            return {};

        if (fullpath.contains(".cnmt") || file->GetSize() > 64_MBYTES) // Skipping large files for now
            return true;
        const auto hashsum{ToArrayOfBytes<16>(std::string_view(fullpath).substr(0, fullpath.find_last_of('.')))};

        thread_local HashSum checksum;
        const auto _filesize{file->GetSize()};
        for (U64 offset{}; offset < _filesize;) {
            const auto size{std::min(file->GetSize() - offset, buffer.size())};
            if (size < buffer.size())
                buffer.resize(size);
            assert(file->ReadSome(buffer, offset) == size); assert(size); // To make sure we're making some progress
            checksum.Update(buffer);
            offset += size;
        }

        const auto result(checksum.Finish());
        fmt::println("SHA256 result of NCA {}, {:x}", GetPathStr(file), fmt::join(result, ""));
        return Contains(hashsum, ToSpan(result));
    }
}
