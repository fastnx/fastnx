#include <common/async_logger.h>
#include <core/assets.h>
#include <core/app_setup.h>

namespace FastNx::Core {
    void FsXmlWriter::write(const void *data, const size_t size) {
        const std::span content{static_cast<const char *>(data), size};
        const std::string_view lines{content.data(), size};

        if (!lines.starts_with("<?xml version=\"1.0\" "))
            return;
        if (file->GetSize() != content.size())
            file->SetSize(content.size());
        file->WriteSome(content);
    }
    void GetDefault(pugi::xml_document &sets) {
        auto xmlbegin{sets.append_child(pugi::node_declaration)};
        xmlbegin.append_attribute("version") = "1.0";
        xmlbegin.append_attribute("encoding") = "UTF-8";
        sets.append_child("FastNx");
    }
    template <typename T>
    void LoadOrSetGlobal(pugi::xml_document &sets, const char *attribute, T &result, T defval) {
        auto globalset{sets.child("FastNx")};
        if (!globalset.attribute(attribute))
            globalset.append_attribute(attribute) = defval;
        result = globalset.attribute(attribute).as_bool();
    }

    AppSetup::AppSetup(const std::shared_ptr<Assets> &assets) :
        sfile(assets->GetFile(AssetFileType::Setupfile)) {
        if (!sfile)
            return;
        try {
            if (!sfile->GetSize())
                GetDefault(xmlset);
            filecontent = sfile->ReadSome<char>(sfile->GetSize());
            if (!filecontent.empty())
                if (const auto &fileerror{xmlset.load_buffer_inplace(filecontent.data(), filecontent.size())}; !fileerror)
                    throw std::format_error{fileerror.description()};
        } catch (const std::exception &parser) {
            AsyncLogger::Error("Failed to load configuration from {} due to {}", FsSys::GetPathStr(sfile), parser.what());
        }

        LoadOrSetGlobal(xmlset, "enable-nsp-formats", enablensps, true);
        WriteToFile();
    }

    void AppSetup::WriteToFile() const {
        if (FsXmlWriter writer{sfile}; xmlset)
            xmlset.save(writer);
    }
    AppSetup::~AppSetup() {
        if (!sfile)
            return;
        WriteToFile();
    }
}
