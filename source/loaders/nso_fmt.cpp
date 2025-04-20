#include <boost/regex.hpp>

#include <common/values.h>
#include <common/async_logger.h>
#include <common/container.h>
#include <common/exception.h>

#include <runtime/lossless.h>

#include <crypto/checksum.h>
#include <loaders/nso_fmt.h>
namespace FastNx::Loaders {
    NsoFmt::NsoFmt(const FsSys::VfsBackingFilePtr &nso, bool &loaded) : AppLoader(nso, loaded, AppType::NsoExe) {
        if (nso->GetSize() < sizeof(NsoHeader))
            return;

        const auto nsofront{nso->Read<NsoHeader>()};
        if (nsofront.magic != ConstMagicValue<U32>("NSO0"))
            return;

        NX_ASSERT(nsofront.modulenameoffset == sizeof(NsoHeader));
        bsssize = nsofront.bsssize;

        const auto modulename{nso->ReadSome<char>(nsofront.modulenamesize, sizeof(NsoHeader))};
        if (strlen(modulename.data()))
            AsyncLogger::Info("NSO module name: {}", std::string_view{modulename.data(), strlen(modulename.data())});

        GetSection(nsofront.textsection, nsofront.textfilesize, NsoSectionType::Text);
        GetSection(nsofront.rosection, nsofront.rofilesize, NsoSectionType::Ro);
        GetSection(nsofront.datasection, nsofront.datafilesize, NsoSectionType::Data);

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
        Finish();
    }

    void NsoFmt::GetSection(const BinarySection &section, const U32 compressed, NsoSectionType type) {
        if (secsmap.contains(type))
            return;
        std::vector<U8> content(section.size);
        if (content.size() == compressed) {
            if (!backing->ReadSome(ToSpan(content), section.fileoffset))
                return;
        } else {
            const std::span output{content.data(), compressed};
            if (backing->ReadSome(output, section.fileoffset) == compressed)
                if (Runtime::FastLz4(ToSpan(content), output) == 0)
                    throw exception{"Failed to decompress the section"};
        }

        if (type == NsoSectionType::Ro)
            if (const std::string rostrs{reinterpret_cast<const char *>(content.data()), content.size()}; !rostrs.empty())
                PrintRo(rostrs);
        secsalign.emplace_back(type, std::make_pair(section.memoryoffset, section.size));
        secsmap.emplace(type, std::move(content));
    }

    std::vector<std::shared_ptr<NsoFmt>> NsoFmt::LoadModules(const FsSys::VfsReadOnlyDirectoryPtr &exefs) {
        // Like the linker, the binaries described in the exefs partition must be loaded in this order, if they exist
        static const std::vector<FsSys::FsPath> exepathlist{"rtld", "main", "subsdk0", "subsdk1", "subsdk2", "subsdk3", "subsdk4", "subsdk5", "subsdk6", "subsdk7", "subsdk8", "subsdk9", "sdk"};

        std::vector<std::shared_ptr<NsoFmt>> nsolist;
        bool loaded{};
        for (const auto &nsofile: exepathlist) {
            if (const auto nso{exefs->OpenFile("exefs" / nsofile)})
                nsolist.emplace_back(std::make_shared<NsoFmt>(nso, loaded));
        }
        return nsolist;
    }

    void NsoFmt::PrintRo(const std::string &strings) const {
        std::optional<std::string_view> modulepath;
        if (std::memcmp(strings.data(), "\\0\\0\\0\\0", 4)) {
            U32 length;
            std::memcpy(&length, &strings[4], sizeof(length));
            if (length)
                modulepath.emplace(&strings[8], length);
        }
        static const boost::regex moduleregex{"[a-z]:[\\/][ -~]{5,}\\.nss", boost::regex::icase};
        boost::smatch match;

        if (!modulepath) {
            if (boost::regex_match(strings, match, moduleregex))
                modulepath.emplace(match.str());
        }
        std::string report;
        report += fmt::format("Module Path: {} ", *modulepath);

        static const boost::regex sdkregex{"SDK MW[ -~]*"};
        static const boost::regex fsregex{"sdk_version: ([0-9.]*)"};
        if (boost::regex_search(strings, match, fsregex))
            report += fmt::format("FS SDK Version: {} ", match.str());

        boost::sregex_iterator itsdk(strings.begin(), strings.end(), sdkregex);
        if (itsdk != decltype(itsdk){})
            report += "SDK Libraries:";
        for (; itsdk != boost::sregex_iterator{}; ++itsdk) {
            report += fmt::format(" {}", itsdk->str().substr(7));
        }

        AsyncLogger::Info("SDK versions of NSO module {}: {}", FsSys::GetPathStr(backing), report);
    }
}
