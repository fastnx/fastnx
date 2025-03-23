#include <common/traits.h>
#include <common/bytes.h>
#include <common/async_logger.h>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <crypto/types.h>
#include <crypto/checksum.h>

namespace FastNx::Crypto {
    bool CheckNcaIntegrity(const FsSys::VfsBackingFilePtr &file) {
        const auto &fullpath{GetPathStr(file)};
        std::vector<U8> buffer(4_MBYTES);

        if (!isalnum(*fullpath.begin()))
            return {};

        if (fullpath.contains(".cnmt") || file->GetSize() > 64_MBYTES) // Skipping large files for now
            return true;
        const auto hashsum{ToArrayOfBytes<16>(std::string_view(fullpath).substr(0, fullpath.find_last_of('.')))};

        thread_local Checksum chk256;
        const auto _filesize{file->GetSize()};
        for (U64 offset{}; offset < _filesize;) {
            const auto size{std::min(file->GetSize() - offset, buffer.size())};
            if (size < buffer.size())
                buffer.resize(size);
            NX_ASSERT(file->ReadSome(std::span(buffer), offset) == size); NX_ASSERT(size); // To make sure we're making some progress
            chk256.Update(buffer);
            offset += size;
        }

        const auto result(chk256.Finish());
        AsyncLogger::Success("SHA256 result of NCA {}, {:x}", GetPathStr(file), fmt::join(result, ""));
        return Contains(hashsum, ToSpan(result));
    }
}
