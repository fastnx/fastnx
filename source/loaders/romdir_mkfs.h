#pragma once

#include <fs_sys/types.h>

namespace FastNx::Loaders {
    enum class ContentType {
        Meta,
        Program,
        Data,
        Control,
        HtmlDocument,
        LegalInformation,
        DeltaFragment
    };

    // https://switchbrew.org/wiki/NCM_services#ContentMetaType
    enum class ContentMetaType {
        Invalid,
        SystemProgram,
        SystemData,
        SystemUpdate,
        BootImagePackage,
        BootImagePackageSafe,
        Application = 0x80,
        Patch,
        AddOnContent,
        Delta,
        DataPatch // [15.0.0+]
    };

    using ContentClassifier = std::pair<ContentType, ContentMetaType>;
    using ContentEnumerate = std::pair<FsSys::FsPath, ContentClassifier>;

    class RomDirMkFs {
    public:
        RomDirMkFs(FsSys::VfsReadOnlyDirectoryPtr &romdir, std::vector<ContentEnumerate> &enums);
    };
}
