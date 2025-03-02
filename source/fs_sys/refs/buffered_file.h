#pragma once

#include <boost/container/small_vector.hpp>
#include <fs_sys/types.h>

namespace FastNx::FsSys::ReFs {
    class BufferedFile final : public VfsBackingFile {
    public:
        explicit BufferedFile(const FsPath &_path, I32 dirfd = {}, AccessModeType _mode = AccessModeType::ReadOnly, bool create = {});
        ~BufferedFile() override;

        explicit operator bool() const override;
        U64 GetSize() const override;
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;

        I32 descriptor{};
        boost::container::small_vector<U8, 8 * 1024> buffer;
        U64 start{};
        U64 valid{}; // Determine the size corresponding to the valid bytes in the buffer
    };
}
