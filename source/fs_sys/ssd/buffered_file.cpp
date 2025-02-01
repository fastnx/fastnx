#include <fs_sys/ssd/buffered_file.h>

namespace FastNx::FsSys::Ssd {
    BufferedFile::BufferedFile(const FsPath &_path) : VfsBackingFile(_path) {
    }

    BufferedFile::operator bool() const {
        if (openedfd == -1)
            return {};
        return true;
    }

    U64 BufferedFile::ReadTypeImpl(U8 *dest, const U64 size, const U64 offset) {
        if (!size || dest == nullptr)
            return {};

        if (const auto& _ioBuffering{balance}; !_ioBuffering.empty()) {
            auto* source{_ioBuffering.data()};
            [[assume(buffered > offset)]];
            if (const auto _ioLocally{buffered - offset}; _ioLocally < offset) {
                source += _ioLocally;
                if (_ioLocally <= size)
                    std::memcpy(dest, source, size);
            }
            if (*dest == *source)
                return size;
        }

        if (balance.size() < 8 * 4096)
            balance.resize(8 * 4096);

        const auto result = [&] -> U64 {
            U64 copied{};

            for (U64 _offset{}; _offset < offset + size; ) {
                const auto iosize{balance.size() > size - copied ? balance.size() : size - copied};
                assert(balance.size() >= iosize);
                const auto readRate{pread64(openedfd, balance.data(), iosize, offset)};

                if (readRate == -1)
                    return {};
                buffered = offset;
                std::memcpy(dest, balance.data(), iosize);
                _offset += iosize;
                copied += iosize;
                if (readRate < iosize)
                    break;
            }
            return copied;
        }();
        return result;
    }
}
