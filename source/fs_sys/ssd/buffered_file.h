#pragma once

#include <boost/container/small_vector.hpp>
#include <fs_sys/types.h>

namespace FastNx::FsSys::Ssd {
    class BufferedFile final : public VfsBackingFile {
    public:
        explicit BufferedFile(const FsPath &_path);

        explicit operator bool() const;

    protected:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;

    public:
        I32 openedfd{-1};
        boost::container::small_vector<U8, 8 * 1024> balance;
        U64 buffered{};
    };
}
