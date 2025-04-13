#include <regex>
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

    std::vector<U8> ReadLogo(const FsSys::VfsBackingFilePtr &file) {
        const auto logo{file->ReadSome(file->GetSize(), 0)};
        const std::regex exif("Nintendo AuthoringTool");
        const std::string content{reinterpret_cast<const char *>(logo.data()), logo.size()};
        if (std::smatch match; std::regex_search(content, match, exif)) {
            return logo;
        }
        return {};
    }
    std::vector<U8> NspEs::GetLogo() {
        std::vector<U8> logo{};
        for (const auto &nca: subnsp->ncalist) {
            for (const auto &romfs: nca->romfslist) {
                if (const auto logofile{romfs->OpenFile("/icon_AmericanEnglish.dat")})
                    logo = ReadLogo(logofile);

            }
        }
        return logo;
    }
    U64 NspEs::GetTitleId() {
        return subnsp->titleid;
    }
}
