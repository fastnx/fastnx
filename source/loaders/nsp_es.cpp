#include <common/async_logger.h>
#include <loaders/nsp_es.h>

namespace FastNx::Loaders {
    NspEs::NspEs(const FsSys::VfsBackingFilePtr &nspf, const std::shared_ptr<Horizon::KeySet> &keys, bool &isLoaded) : AppLoader(nspf, isLoaded, AppType::NspEs),
        _mainPfs(std::make_shared<FsSys::NxFmt::PartitionFileSystem>(nspf)) {
        if (!IsAValidPfs(_mainPfs)) {
            status = LoaderStatus::PfsNotFound;
            return;
        }

        AsyncLogger::Puts("Files in this PFS: \n");
        for (const auto &partfile: _mainPfs->ListAllFiles()) {
            AsyncLogger::Puts("- {}\n", FsSys::GetPathStr(partfile));
        }

        subnsp = std::make_shared<FsSys::NxFmt::SubmissionPackage>(_mainPfs, keys);

        status = LoaderStatus::Success;
        Finish();
    }
}
