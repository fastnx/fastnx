#pragma once

#include <fs_sys/refs/huge_file.h>
#include <fs_sys/types.h>

namespace FastNx::FsSys::Vfs {
    class OffsetFile final : public VfsBackingFile {
    public:
        OffsetFile(const VfsBackingFilePtr &backing, const FsPath &_path, U64 offset, U64 size, bool isHuge = {});

        explicit operator bool() const override;
        U64 GetSize() const override;
        void SetSize(U64 newsize) override;
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;

        U64 size{};
        U64 begin{};
        U64 eof{};

        std::shared_ptr<ReFs::HugeFile> hugefs{};
        VfsBackingFilePtr _source{};
    };
}
