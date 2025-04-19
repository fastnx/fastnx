#include <vector>
#include <lz4.h>

#include <common/container.h>
#include <runtime/lossless.h>
namespace FastNx::Runtime {
    U64 FastLz4(const std::span<char> &dest, const std::span<const char> &source) {
        if (dest.data() != source.data()) {
            if (const auto requested{LZ4_decompress_safe(source.data(), nullptr, source.size(), 0)}; requested > 0) {
                if (dest.size() != static_cast<U64>(requested))
                    return LZ4_decompress_safe(source.data(), dest.data(), source.size(), requested);
            }
            return {};
        }
        std::vector<char> buffer(32_KBYTES);

        U64 destoffset{};
        auto *context{LZ4_createStreamDecode()};
        for (U64 sourceoffset{}; sourceoffset < source.size(); ) {
            const auto size{std::min(buffer.size(), source.size() - sourceoffset)};
            std::memcpy(buffer.data(), source.data() + sourceoffset, size);

            if (const auto increase{LZ4_decompress_safe_continue(context, buffer.data(), dest.data(), size, dest.size())}; increase > 0) {
                destoffset += increase;
                sourceoffset += size;
            } else {
                break;
            }
        }
        LZ4_freeStreamDecode(context);
        return destoffset;
    }
}
