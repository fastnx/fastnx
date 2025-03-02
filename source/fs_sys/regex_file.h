#pragma once

#include <fs_sys/types.h>
namespace FastNx::FsSys {
    class RegexFile final : public VfsBackingFile {
    public:
        explicit RegexFile(const FsPath &_path, const std::string &pattern);

        explicit operator bool() const override;
        U64 GetSize() const override;
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;

        VfsBackingFilePtr file{};
        std::vector<std::string> matches;
    };
}