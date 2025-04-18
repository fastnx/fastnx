#include <fs_sys/npdm.h>
#include <common/async_logger.h>
#include <loaders/nsp_es.h>


namespace FastNx::Loaders {
    NspEs::NspEs(const FsSys::VfsBackingFilePtr &nspf, const std::shared_ptr<Horizon::KeySet> &keys, bool &isloaded) : AppLoader(nspf, isloaded, AppType::NspEs),
        _mainpfs(std::make_shared<FsSys::NxFmt::PartitionFileSystem>(nspf)) {
        if (!IsAValidPfs(_mainpfs)) {
            status = LoaderStatus::PfsNotFound;
            return;
        }

        AsyncLogger::Puts("Files in this PFS: \n");
        for (const auto &partfile: _mainpfs->ListAllFiles()) {
            AsyncLogger::Puts("- {}\n", FsSys::GetPathStr(partfile));
        }

        subnsp = std::make_shared<FsSys::NxFmt::SubmissionPackage>(_mainpfs, keys);

        status = LoaderStatus::Success;
        Finish();
    }

    void NspEs::LoadApplication() {
        [[maybe_unused]] FsSys::Npdm pMeta{subnsp->appdir->GetNpdm()};
    }

    std::vector<U8> ReadLogo(const FsSys::VfsBackingFilePtr &file) {
        const auto logo{file->ReadSome(file->GetSize())};
        const std::string jpegtiff{"Nintendo AuthoringTool"};
        static std::vector<U8> authority;
        if (authority.empty()) {
            authority.resize(jpegtiff.size());
            std::memcpy(authority.data(), jpegtiff.data(), jpegtiff.size());
        }
        return !std::ranges::search(logo, authority).empty() ? logo : std::vector<U8>{};
    }

    std::vector<U8> NspEs::GetLogo() {
        if (const auto &nca{subnsp->GetContentNca(FsSys::LogoType)}) {
            for (const auto &romfs: nca->romfslist) {
                if (const auto logofile{romfs->OpenFile("icon_AmericanEnglish.dat")})
                    return ReadLogo(logofile);

            }
        }
        return {};
    }
    U64 NspEs::GetTitleId() {
        return subnsp->titleid;
    }
}
