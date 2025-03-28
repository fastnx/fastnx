#include <fs_sys/types.h>

namespace FastNx::FsSys {
    std::vector<FsPath> VfsReadOnlyDirectory::GlobAllFiles(const std::string &pattern, const bool followTree) const {
        auto filter{pattern};
        const bool starts{filter.ends_with('*')};
        if (const auto starfish{filter.find('*')}; starfish != std::string::npos)
            filter.erase(starfish, starfish + 1);
        else
            filter.clear();

        std::vector<FsPath> filtered; {
            for (const auto files{followTree ? ListAllFiles() : ListAllTopLevelFiles()}; const auto &_path: files) {
                if (const auto &filename{_path.filename()}; _path.has_filename()) {
                    if ((starts && filename.string().starts_with(filter)) || (!starts && filename.string().ends_with(filter)))
                        filtered.emplace_back(_path);
                }
            }
        }
        return filtered;
    }
}
