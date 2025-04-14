#pragma once

#include <fs_sys/types.h>
namespace FastNx::FsSys::Vfs {
    class RegexFile final : public VfsBackingFile {
    public:
        explicit RegexFile(const VfsBackingFilePtr &_file, const std::string &pattern);

        explicit operator bool() const override;
        U64 GetSize() const override;

        auto GetAllMatches() {
            file = nullptr;
            return std::move(matches);
        }
    private:
        std::vector<std::string> matches;
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;

        VfsBackingFilePtr file{};
    };
}