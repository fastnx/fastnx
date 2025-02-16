#pragma once

#include <fs_sys/types.h>

namespace FastNx::FsSys::ReFs {
    class HugeFile final : public VfsBackingFile {
    public:
        explicit HugeFile(const FsPath &_path, AccessModeType _mode = AccessModeType::ReadOnly);
        ~HugeFile() override;
        explicit operator bool() const;

    protected:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;

    private:
        I32 openedfd{-1};
        U8 *memory{nullptr};
        U8 *recorded{nullptr};
        U64 mapsize{};

        U64 pagemissRec{};
    };
}
