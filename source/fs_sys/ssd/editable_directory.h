#pragma once

#include <fs_sys/types.h>
namespace FastNx::FsSys::Ssd {
    class EditableDirectory final : public VfsBackingDirectory {
    public:
        explicit EditableDirectory(const FsPath &_path, bool create = {});
        std::vector<FsPath> ListAllFiles() override;

        explicit operator bool() const;

        int descriptor;
    };
}
