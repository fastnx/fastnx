#include <cassert>
#include <loaders/nsp_es.h>

namespace FastNx::Loaders {
    NspEs::NspEs(const FsSys::VfsBackingFilePtr &nspf, bool &isLoaded) : AppLoader(nspf, isLoaded, AppType::NspEs),
        _mainPfs(std::make_shared<FsSys::NxFmt::PartitionFileSystem>(nspf)) {
        if (!IsAValidPfs(_mainPfs))
            status = LoaderStatus::PfsNotFound;

        status = LoaderStatus::Success;
        _Finish();
        assert(isLoaded);
    }
}
