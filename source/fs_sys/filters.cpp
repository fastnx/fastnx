#include <fs_sys/types.h>

namespace FastNx::FsSys {
    std::vector<FsPath> VfsReadOnlyDirectory::GlobAllFiles(const std::string &pattern, const bool followTree) const {
        auto filter{pattern};
        const bool starts{filter.ends_with('*')};
        if (const auto starfish{filter.find('*')}; starfish != std::string::npos)
            filter.erase(starfish, starfish + 1);
        else
            filter.clear();

        const bool isfilename{!filter.contains('/')};
        std::vector<FsPath> filtered; {
            auto TestFilter = [&](const auto &path) {
                return (starts && path.string().starts_with(filter)) || (!starts && path.string().ends_with(filter));
            };

            for (const auto files{followTree ? ListAllFiles() : ListAllTopLevelFiles()}; const auto &filepath: files) {
                if (const auto &filename{isfilename ? filepath.filename() : filepath}; !filepath.empty()) {
                    if (TestFilter(filename))
                        filtered.emplace_back(filepath);
                }
            }
        }
        return filtered;
    }
}
