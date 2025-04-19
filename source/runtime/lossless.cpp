#include <vector>
#include <lz4.h>

#include <common/container.h>
#include <runtime/lossless.h>
namespace FastNx::Runtime {
    U64 FastLz4(const std::span<char> &dest, const std::span<const char> &source) {
        std::vector<char> buffer;
        const auto size{source.size()};
        if (dest.data() == source.data()) {
            buffer.resize(size);
            std::memcpy(buffer.data(), source.data(), size);
        }

        const auto *sourcebuf{buffer.empty() ? source.data() : buffer.data()};
        // Why does LZ4_compressBound always return a value smaller than the required output size?
        if (static_cast<U64>(LZ4_compressBound(size)) <= dest.size())
            if (const auto result{LZ4_decompress_safe(sourcebuf, dest.data(), source.size(), dest.size())}; result > 0)
                return result;

        std::memset(dest.data(), 0, dest.size());
        return {};
    }
}
