#include <print>

#include <crypto/types.h>
#include <common/async_logger.h>
#include <common/container.h>
#include <common/exception.h>
#include <fs_sys/linkable_directory.h>
#include <fs_sys/nx_fmt/submission_package.h>

#include <loaders/nsp_es.h>

namespace FastNx::FsSys::NxFmt {
    SubmissionPackage::SubmissionPackage(Loaders::NspEs &nsp, const std::shared_ptr<Horizon::KeySet> &_keys) : pfs(nsp.mainpfs), keys(_keys)  {
        auto files{pfs->ListAllFiles()};

        for (auto subit{files.begin()}; subit != files.end() && !cnmt; ++subit) {
            bool erase{};
            if (subit->extension() == ".tik") {
                keys->AddTicket(pfs->OpenFile(*subit));
                erase = true;
            }
            if (const auto &subpath{subit->stem()}; subit->has_stem())
                if (subpath.has_extension() && subpath.extension() == ".cnmt") {
                    const auto cnmtnca{std::make_unique<ContentArchive>(pfs->OpenFile(*subit), keys)};
                    if (const auto pfsinner{cnmtnca->pfslist.front()}; (cnmt = pfsinner->OpenFile(pfsinner->ListAllFiles().front())))
                        erase = true;
                }

            if (erase)
                files.erase(subit);
        }

        GetAll(nsp, cnmt, files);

        if (corrupted)
            AsyncLogger::Error("The NCA file {} is corrupted, check your ROM", GetPathStr(corrupted));
    }

    void SubmissionPackage::GetAll(Loaders::NspEs &nsp, const VfsBackingFilePtr &cnmt, const std::vector<FsPath> &content) {
        NX_ASSERT(cnmt && cnmt->GetSize() > 0);

        Cnmt categorizer(cnmt);

        for (const auto &validfile: content) {
            if (corrupted || validfile.extension() != ".nca")
                continue;
            if (const auto ncafile{pfs->OpenFile(validfile)}) {
                if (!Crypto::CheckNcaIntegrity(ncafile))
                    corrupted = ncafile;

                AsyncLogger::Info("Processing content of NCA {}", GetPathStr(ncafile));
                const auto nca{std::make_shared<ContentArchive>(std::move(ncafile), keys)};
                if (!titleid && nca->type == ContentType::Program)
                    titleid = nca->titleid;

                ncalist.emplace(std::make_pair(categorizer.type, nca->type), std::move(nca));
            }
        }

        NX_ASSERT(ncalist.begin()->second->size > 0);

        nsp.appdir = [&] -> std::shared_ptr<Loaders::ApplicationDirectory> {
            auto directory{std::make_shared<LinkableDirectory>(std::format("{:X}", titleid))};
            if (!directory)
                return nullptr;

            if (std::vector<ContentEnumerate> livedata; Populate(directory, livedata))
                return std::make_shared<Loaders::ApplicationDirectory>(std::move(directory), std::move(livedata));
            return nullptr;
        }();

#if !NDEBUG
        // appdir->ExtractAllFiles();
#endif
    }

    std::shared_ptr<ContentArchive> SubmissionPackage::GetContentNca(const ContentClassifier &type) {
        if (const auto content{ncalist.find(type)}; content != ncalist.end())
            return content->second;
        return nullptr;
    }

    std::optional<FsPath> GetAppDirectoryDir(const ContentClassifier &type, const std::vector<FsPath> &files) {
        if (type.second == ContentType::Data)
            return "romfs";
        if (Contains(files, {"NintendoLogo.png", "StartupMovie.gif"}))
            return "logo";
        for (const auto &filename: files) {
            if (filename.extension() == ".cnmt")
                return "meta";
            if (filename == "control.nacp")
                return "exefs";
        }
        if (type.second == ContentType::Program)
            return Contains(files, "main") ? "exefs" : "romfs";
        return std::nullopt;
    }

    bool SubmissionPackage::Populate(const std::shared_ptr<LinkableDirectory> &directory, std::vector<ContentEnumerate> &meta) {
        auto Callback = [&](const VfsReadOnlyDirectoryPtr &vfs, const ContentClassifier &type) -> bool {

            auto CreateLink = [&](const FsPath &filepath, const VfsBackingFilePtr &file) {
                directory->LinkFile(filepath, file);
                meta.emplace_back(ContentEnumerate{filepath, type});
            };
            const auto &files{vfs->ListAllFiles()};
            if (files.empty())
                return {};

            if (const auto outputdir{GetAppDirectoryDir(type, files)}) {
                for (const auto &file: files)
                    CreateLink(*outputdir / file, vfs->OpenFile(file));
                return true;
            }
            return {};
        };

        // https://switchbrew.org/wiki/NCA_Content_FS#NCA-type0
        auto GetContent = [&](const std::shared_ptr<ContentArchive> &nca, const ContentClassifier &type) -> bool {
            for (const auto &partition: nca->pfslist)
                if (!Callback(partition, type))
                    return {};
            for (const auto &romfs: nca->romfslist)
                if (!Callback(romfs, type))
                    return {};
            return true;
        };

        for (const auto info: EnumRange(ContentType::Program, ContentType::PublicData)) {
            const ContentClassifier type{ContentMetaType::Application, info};
            if (const auto &nca{GetContentNca(type)})
                if (!GetContent(nca, type))
                    if (type.second == ContentType::Program)
                        throw exception{"Failed to extract all files"};
        }

        return true;
    }
}
