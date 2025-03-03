#include <print>

#include <crypto/types.h>
#include <fs_sys/nx_fmt/submission_package.h>
namespace FastNx::FsSys::NxFmt {
    SubmissionPackage::SubmissionPackage(const std::shared_ptr<PartitionFileSystem> &pfs) {
        auto files{pfs->ListAllFiles()};

        for (auto subit{files.begin()}; subit != files.end() && !cnmt; ++subit) {
            if (!subit->stem().has_extension())
                continue;
            if ((cnmt = pfs->OpenFile(*subit)))
                files.erase(subit);
        }

        assert(cnmt && cnmt->GetSize() > 0);
        for (const auto &content : files) {
            assert(content.extension() == ".nca");
            if (corrupted) {
                std::println("The NCA file {} is corrupted, check your ROM", corrupted->path.string());
                return;
            }
            if (const auto ncafile{pfs->OpenFile(content)}) {
                if (Crypto::CheckNcaIntegrity(ncafile) == false)
                    corrupted = ncafile;
            }
        }
    }
}
