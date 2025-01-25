#pragma once

#include <fs_sys/types.h>
namespace FastNx::FsSys::SSD {
    class SolidDirectory {
    public:
        explicit SolidDirectory(const FsPath &_path, bool create = {});
        FsPath path;
        int descriptor;
    };
}
