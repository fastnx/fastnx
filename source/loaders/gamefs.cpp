#include <algorithm>

#include <common/async_logger.h>
#include <loaders/gamefs.h>
namespace FastNx::Loaders {
    GameFileSystem::GameFileSystem(const FsSys::VfsReadOnlyDirectoryPtr &romdir, const std::vector<ContentEnumerate> &enums) {
        NX_ASSERT(romdir->GetFilesCount());
        std::ranges::for_each(enums, [](const auto &files) {
            AsyncLogger::Info("{}", FsSys::GetPathStr(files.first));
        });
    }
}
