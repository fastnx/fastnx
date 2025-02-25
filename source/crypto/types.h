#pragma once

#include <fs_sys/types.h>

namespace FastNx::Crypto {
    bool CheckNcaIntegrity(const FsSys::VfsBackingFilePtr &file);
}
