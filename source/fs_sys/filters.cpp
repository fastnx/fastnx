#include <fs_sys/types.h>

namespace FastNx::FsSys {
    std::vector<FsPath> VfsBackingDirectory::BlobAllFiles(const std::string &pattern) {
        auto filter{pattern};
        if (const auto starfish{filter.find('*')}; starfish != std::string::npos)
            filter.erase(starfish, starfish + 1);
        else
            filter.clear();

        std::vector<FsPath> filtered; {
            for (auto files{ListAllFiles()}; const auto &_path: files) {
                if (_path.string().contains(filter))
                    filtered.emplace_back(std::move(_path));
            }
        }
        return filtered;
    }
}
