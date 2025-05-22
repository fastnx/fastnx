#pragma once
#include <pugixml.hpp>
#include <fs_sys/types.h>

namespace FastNx::Core {
    class Assets;

    class FsXmlWriter final : public pugi::xml_writer {
    public:
        explicit FsXmlWriter(const FsSys::VfsBackingFilePtr &_file) : file(_file) {}
        void write(const void *data, size_t size) override;

        const FsSys::VfsBackingFilePtr &file;
    };

    class AppSetup {
    public:
        explicit AppSetup(const std::shared_ptr<Assets> &assets);
        ~AppSetup();

        FsSys::VfsBackingFilePtr sfile;
        pugi::xml_document rawsettings;
    };
}
