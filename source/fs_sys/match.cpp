#include <common/container.h>
#include <crypto/checksum.h>
#include <fs_sys/types.h>

namespace FastNx::FsSys {
    bool MatchFiles(const VfsBackingFilePtr &first, const VfsBackingFilePtr &second) {
        // Simple and cheap test
        if (first->GetSize() != second->GetSize())
            return {};
        if (first->Read<U64>() != second->Read<U64>())
            return {};
        const auto flatsize{first->GetSize()};
        std::vector<U8> buffer(std::min(flatsize, 128_KBYTES));

        // The heavy part comes here
        std::array<Crypto::Checksum, 2> checkers;
        for (U64 offset{}; offset < first->GetSize(); ) {
            if (const auto bufsz{std::min(buffer.size(), flatsize - offset)})
                if (bufsz != buffer.size())
                    buffer.resize(bufsz);
            const auto size{first->ReadSome(std::span(buffer), offset)};
            if (size)
                checkers.front().Update(buffer);
            else break;
            if (second->ReadSome(std::span(buffer), offset) == size)
                checkers.back().Update(buffer);

            offset += size;
        }
        std::array<std::array<U8, 32>, 2> hashes;
        for (const auto& [hasher, result]: std::views::zip(checkers, hashes)) {
            hasher.Finish(result);
        }

        return IsEqual(hashes[0], hashes[1]);
    }
}
