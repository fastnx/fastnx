#include <cassert>
#include <print>
#include <loaders/nsp_es.h>

namespace FastNx::Loaders {
    NspEs::NspEs(const FsSys::VfsBackingFilePtr &nspf, const std::shared_ptr<Horizon::KeySet> &keys, bool &isLoaded) : AppLoader(nspf, isLoaded, AppType::NspEs),
        _mainPfs(std::make_shared<FsSys::NxFmt::PartitionFileSystem>(nspf)) {
        if (!IsAValidPfs(_mainPfs)) {
            status = LoaderStatus::PfsNotFound;
            return;
        }

        std::puts("Files in this PFS: ");
        for (const auto &partfile: _mainPfs->ListAllFiles()) {
            std::println("- {}", FsSys::GetPathStr(partfile));
        }

        subnsp = std::make_shared<FsSys::NxFmt::SubmissionPackage>(_mainPfs, keys);

        status = LoaderStatus::Success;
        Finish();
    }
}
