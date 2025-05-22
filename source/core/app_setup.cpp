#include <core/assets.h>
#include <core/app_setup.h>

namespace FastNx::Core {
    void FsXmlWriter::write(const void *data, const size_t size) {
        file->Write(data, size);
    }

    AppSetup::AppSetup(const std::shared_ptr<Assets> &assets) :
        sfile(assets->GetFile(AssetFileType::Setupfile)) {
        if (!sfile || sfile->GetSize())
            return;
        auto xmlset{sfile->ReadSome(sfile->GetSize())};

        rawsettings.load_buffer_inplace(xmlset.data(), xmlset.size());
    }
    AppSetup::~AppSetup() {
        if (!sfile)
            return;
        FsXmlWriter writer{sfile};
        if (rawsettings)
            rawsettings.save(writer);
    }
}
