#include <print>

#include <crypto/types.h>
#include <common/async_logger.h>
#include <fs_sys/nx_fmt/submission_package.h>


namespace FastNx::FsSys::NxFmt {
    SubmissionPackage::SubmissionPackage(const std::shared_ptr<PartitionFileSystem> &pfs, const std::shared_ptr<Horizon::KeySet> &keys) {
        auto files{pfs->ListAllFiles()};

        for (auto subit{files.begin()}; subit != files.end() && !cnmt; ++subit) {
            bool erase{};
            if (subit->extension() == ".tik") {
                keys->AddTicket(pfs->OpenFile(*subit));
                erase = true;
            }
            if (const auto &subpath{subit->stem()}; subit->has_stem())
                if (subpath.has_extension() && subpath.extension() == ".cnmt")
                    if ((cnmt = pfs->OpenFile(*subit)))
                        erase = true;

            if (erase)
                files.erase(subit);
        }

        NX_ASSERT(cnmt && cnmt->GetSize() > 0);
        for (const auto &content: files) {
            if (content.extension() != ".nca")
                continue;
            if (const auto ncafile{pfs->OpenFile(content)}) {
                if (!Crypto::CheckNcaIntegrity(ncafile)) {
                    corrupted = ncafile;
                    break;
                }

                AsyncLogger::Info("Processing content of NCA {}", GetPathStr(ncafile));
                const auto nca{std::make_shared<ContentArchive>(std::move(ncafile), keys)};
                NX_ASSERT(nca->size);
                if (!titleid && nca->type == ContentType::Program)
                    titleid = nca->titleid;

                ncalist.emplace_back(std::move(nca));
            }
        }
        if (corrupted)
            AsyncLogger::Error("The NCA file {} is corrupted, check your ROM", GetPathStr(corrupted));
    }
}
