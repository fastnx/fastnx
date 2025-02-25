#include <common/traits.h>
#include <common/container.h>
#include <crypto/types.h>

namespace FastNx::Crypto {
    template<U64 Size, typename T> requires (IsStringType<T>)
    constexpr std::array<U8, Size> ToArrayOfBytes(const T &string) {
        std::array<U8, Size> result{};
        if (string.size() / 2 != result.size())
            throw std::bad_cast();

        std::memcpy(result.data(), string.data(), result.size());
        return result;
    }

    bool CheckNcaIntegrity(const FsSys::VfsBackingFilePtr &file) {
        const auto &fullpath{file->path.string()};
        std::vector<U8> buffer(4_MEGAS);

        if (!isalnum(*fullpath.begin()))
            return {};

        const auto hashsum{ToArrayOfBytes<16>(fullpath)};

        for (U64 offset{}; offset < file->GetSize();) {
            const auto size{std::min(file->GetSize() - offset, buffer.size())};
            assert(file->ReadSome(buffer, offset) == size);
            offset += size;
        }

        std::array<U8, 16> result{};
        std::memcpy(result.data(), buffer.data(), result.size());
        return IsEqual(hashsum, ToSpan(result));
    }
}
