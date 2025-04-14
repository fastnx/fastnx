#pragma once

#include <common/types.h>
#include <fs_sys/types.h>
namespace FastNx::FsSys::Vfs {
    class StandardFile final : public VfsBackingFile {
    public:
        explicit StandardFile(std::ostream& stream);

        explicit operator bool() const override;

        U64 GetSize() const override;
    private:
        U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) override;
        U64 WriteTypeImpl(const U8 *source, U64 size, U64 offset) override;
        U64 _size{};
        std::ostream &outstr;
    };
}