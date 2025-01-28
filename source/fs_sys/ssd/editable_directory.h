#pragma once

#include <fs_sys/types.h>
namespace FastNx::FsSys::SSD {
    class EditableDirectory final : public VfsBackingDirectory {
    public:
        explicit EditableDirectory(const FsPath &_path, bool create = {});
        std::vector<FsPath> ListAllFiles() override;

        operator bool() const;

        int descriptor;
    };
}
