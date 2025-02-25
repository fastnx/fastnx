#include <crypto/types.h>
#include <fs_sys/nx_fmt/submission_package.h>

namespace FastNx::FsSys::NxFmt {
    SubmissionPackage::SubmissionPackage(const std::shared_ptr<PartitionFileSystem> &pfs) {
        const auto files{pfs->ListAllFiles()};

        std::ranges::for_each(files, [](const auto &content) {
            assert(content.extension() == ".nca");
            Crypto::CheckNcaIntegrity(nullptr);
        });

        [[maybe_unused]] std::vector<Loaders::ContentClassifier> classes{};
    }
}
