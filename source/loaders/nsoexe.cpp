#include <common/values.h>
#include <common/async_logger.h>
#include <common/container.h>
#include <common/exception.h>

#include <crypto/checksum.h>
#include <loaders/nsoexe.h>
namespace FastNx::Loaders {


    NsoExe::NsoExe(const FsSys::VfsBackingFilePtr &nso, bool &loaded) : AppLoader(nso, loaded, AppType::NsoExe) {
        if (nso->GetSize() < sizeof(NsoHeader))
            return;

        const auto nsofront{nso->Read<NsoHeader>()};
        if (nsofront.magic != ConstMagicValue<U32>("NSO0"))
            return;

        NX_ASSERT(nsofront.modulenameoffset == sizeof(NsoHeader));

        const auto modulename{nso->ReadSome<char>(nsofront.modulenamesize, sizeof(NsoHeader))};
        if (strlen(modulename.data()))
            AsyncLogger::Info("NSO module name: {}", std::string_view{modulename.data(), strlen(modulename.data())});

        GetSection(nsofront.textsection, nsofront.textfilesize, NsoSectionType::Text);
        GetSection(nsofront.datasection, nsofront.datafilesize, NsoSectionType::Data);
        GetSection(nsofront.rosection, nsofront.rofilesize, NsoSectionType::Ro);

        if ((nsofront.flags & 1 << 3) == 0)
            return;

        Crypto::Checksum checksum;
        for (auto itsec{secsmap.begin()}; itsec != secsmap.end(); ++itsec) {
            const auto &sectionhash{nsofront.sectionshash[std::to_underlying(itsec->first)]};

            if (const auto &content{itsec->second}; !content.empty())
                checksum.Update(content.data(), content.size());
            if (const auto result{checksum.Finish()}; !IsEqual(result, sectionhash))
                throw exception("NSO section {} appears to be corrupted", FsSys::GetPathStr(nso));
        }
    }

    void NsoExe::LoadApplication(std::shared_ptr<Kernel::Types::KProcess> &kprocess) {}

    std::vector<U8> NsoExe::GetLogo() {
        return {};
    }
    U64 NsoExe::GetTitleId() {
        return {};
    }

    void NsoExe::GetSection(const BinarySection &section, const U32 compressed, NsoSectionType type) {
        std::vector<U8> content(section.size);
        if (!backing->ReadSome(std::span{content.data(), compressed}, section.fileoffset))
            return;

        if (content.size() != compressed) {}
        secsmap.emplace(type, std::move(content));
    }

    void NsoExe::LoadModules([[maybe_unused]] const std::shared_ptr<Kernel::Types::KProcess> &kprocess, const FsSys::VfsReadOnlyDirectoryPtr &exefs) {

        // Like the linker, the binaries described in the exefs partition must be loaded in this order, if they exist
        static const std::vector<FsSys::FsPath> exepathlist{"rtld", "main", "subsdk0", "subsdk1", "subsdk2", "subsdk3", "subsdk4", "subsdk5", "subsdk6", "subsdk7", "subsdk8", "subsdk9", "sdk"};

        std::vector<std::shared_ptr<NsoExe>> nsolist;
        bool loaded{};
        for (const auto &nsofile: exepathlist) {
            if (const auto nso{exefs->OpenFile("exefs" / nsofile)})
                nsolist.emplace_back(std::make_shared<NsoExe>(nso, loaded));
        }
    }

}
