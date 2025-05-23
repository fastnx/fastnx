#pragma once

#include <fs_sys/types.h>

namespace FastNx::FsSys::ReFs {
    class HugeFile final : public VfsBackingFile {
    public:
        explicit HugeFile(const FsPath &_path, I32 dirfd = {}, FileModeType _mode = FileModeType::ReadOnly);
        ~HugeFile() override;
        explicit operator bool() const override;

        U64 GetSize() const override;
        U64 ReadTypeFaster(U8 *dest, U64 size, U64 offset);
        void SetSize(U64 newsize) override;
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;

        I32 descriptor{};
        U8 *memory{};
        U8 *recorded{};
        U64 mapsize{};

        U64 pagefaultrec{};
    };
}
