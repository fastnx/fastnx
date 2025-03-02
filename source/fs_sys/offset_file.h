#pragma once

#include <fs_sys/refs/huge_file.h>
#include <fs_sys/types.h>

namespace FastNx::FsSys {
    class OffsetFile final : public VfsBackingFile {
    public:
        OffsetFile(const VfsBackingFilePtr &backing, const FsPath &_path, U64 offset, U64 size, bool isHuge = {});

        operator bool() const override;
        U64 GetSize() const override;
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;

        U64 size{};
        U64 begin{};
        U64 eof{};

        std::shared_ptr<ReFs::HugeFile> hugefs{};
        VfsBackingFilePtr _source{};
    };
}
