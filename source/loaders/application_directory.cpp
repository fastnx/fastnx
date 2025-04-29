#include <algorithm>
#include <fstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#include <common/container.h>
#include <common/async_logger.h>
#include <common/exception.h>
#include <fs_sys/refs/directory_file_access.h>
#include <fs_sys/refs/buffered_file.h>

#include <fs_sys/linkable_directory.h>
#include <loaders/application_directory.h>



namespace FastNx::Loaders {
    bool ValidDirectoryFiles(const std::vector<FsSys::FsPath> &files) {
        const std::vector<FsSys::FsPath> appfiles{"exefs/main.npdm", "exefs/main"};
        // "version.txt", "autofiles.txt"
        return std::ranges::includes(files, appfiles);
    }
    ApplicationDirectory::ApplicationDirectory(const FsSys::VfsReadOnlyDirectoryPtr &content, std::vector<FsSys::ContentEnumerate> &&metadata) : appdir(content), contentenum(std::move(metadata)) {
        if (!content->GetFilesCount())
            return;
        if (!ValidDirectoryFiles(content->ListAllFiles()))
            return;
    }

    FsSys::ContentClassifier GetType(const std::string_view &line) {
        FsSys::ContentMetaType meta;
        FsSys::ContentType type;

        std::vector<std::string_view> tokens;
        boost::split(tokens, line, boost::is_any_of(","));
        for (const auto &identifer: tokens) {
            if (identifer == "app")
                meta = FsSys::ContentMetaType::Application;
            if (identifer == "control")
                type = FsSys::ContentType::Control;
            if (identifer == "exe")
                type = FsSys::ContentType::Program;
        }

        return std::make_pair(meta, type);
    }
    std::string GetType(const FsSys::ContentClassifier &type) {
        std::string redable;
        if (type.first == FsSys::ContentMetaType::Application)
            redable += "app";

        switch (type.second) {
            case FsSys::ContentType::Control:
                redable += ",control";
                break;
            case FsSys::ContentType::Program:
                redable += ",exe";
                break;
            default: {}
        }
        return redable;
    }

    ApplicationDirectory::ApplicationDirectory(const FsSys::VfsReadOnlyDirectoryPtr &files) : appdir(files) {
        if (!files->OpenFile("version.txt"))
            return;

        const auto autofiles{files->OpenFile("autofiles.txt")};
        contentenum.reserve(files->GetFilesCount());
        if (const auto line{autofiles->ReadLine()}; !line.empty())
            if (const auto _type{GetType(line)}; _type.first != FsSys::ContentMetaType::Invalid)
                contentenum.emplace_back(line.substr(line.find(';') + 1, line.size()),  _type);
    }

    void ApplicationDirectory::ExtractAllFiles() const {
        const auto tmp{std::filesystem::temp_directory_path() / appdir->path};
        std::filesystem::create_directories(tmp);
        std::fstream version{tmp / "version.txt", std::ios::app};
        std::fstream files{tmp / "autofiles.txt", std::ios::app};

        version << "100";

        NX_ASSERT(contentenum.size() == appdir->GetFilesCount());

        std::vector<U8> buffer(8_MBYTES);
        for (const auto &[filepath, _type]: contentenum) {
            files << fmt::format("{};{}\n", GetType(_type), FsSys::GetPathStr(filepath));

            std::filesystem::create_directories(tmp / filepath.parent_path());
            auto outputfile{std::make_unique<FsSys::ReFs::BufferedFile>(tmp / filepath, 0, FsSys::FileModeType::WriteOnly, true)};

            const auto inputfile{appdir->OpenFile(filepath)};
            for (U64 offset{}; offset < inputfile->GetSize(); ) {
                const auto slice{std::min(inputfile->GetSize() - offset, buffer.size())};
                if (inputfile->ReadSome(std::span{buffer.data(), slice}, offset) != slice)
                    throw exception{"Failed to read all {} bytes at offset {} from file {}", slice, offset, FsSys::GetPathStr(filepath)};

                outputfile->WriteSome(std::span{buffer.data(), slice}, offset);
                offset += slice;
            }
        }
    }

    FsSys::VfsBackingFilePtr ApplicationDirectory::GetNpdm() const {
        if (appdir->Exists("exefs/main.npdm"))
            return appdir->OpenFile("exefs/main.npdm");
        return nullptr;
    }

    FsSys::VfsReadOnlyDirectoryPtr ApplicationDirectory::GetExefs() const {
        const auto exefs{std::make_shared<FsSys::LinkableDirectory>("exefs")};

        const auto files{appdir->GlobAllFiles("exefs/*", true)};
        for (const auto &exefile: files)
            exefs->LinkFile(exefile, appdir->OpenFile(exefile));
        return exefs;
    }

    bool IsApplicationDirectory(const FsSys::FsPath &dirfs) {
        NX_ASSERT(is_directory(dirfs));
        FsSys::ReFs::DirectoryFileAccess directory{dirfs};
        const auto version{directory.OpenFile("version.txt")};
        if (!version)
            return {};

        if (const auto line{version->ReadLine()}; !line.empty())
            if (std::strtoul(line.data(), nullptr, 10) < 100)
                return {};

        if (const auto content{directory.ListAllFiles()}; !content.empty())
            if (ValidDirectoryFiles(content))
                return true;
        return {};
    }
}
