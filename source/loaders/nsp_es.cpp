#include <cassert>
#include <print>
#include <loaders/nsp_es.h>

namespace FastNx::Loaders {
    NspEs::NspEs(const FsSys::VfsBackingFilePtr &nspf, bool &isLoaded) : AppLoader(nspf, isLoaded, AppType::NspEs),
        _mainPfs(std::make_shared<FsSys::NxFmt::PartitionFileSystem>(nspf)) {
        if (!IsAValidPfs(_mainPfs)) {
            status = LoaderStatus::PfsNotFound;
            return;
        }

        std::puts("Files in this PFS: ");
        for (const auto &partfile: _mainPfs->ListAllFiles()) {
            std::println("- {}", partfile.string());
        }

        status = LoaderStatus::Success;
        _Finish();
        assert(isLoaded);
    }
}
