#pragma once

#include <fs_sys/types.h>

namespace FastNx::FsSys {
    enum class ContentType : U8 {
        Program,
        Meta,
        Control,
        Manual,
        Data,
        PublicData
    };

    // https://switchbrew.org/wiki/NCM_services#ContentMetaType
    enum class ContentMetaType : U8 {
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

    using ContentClassifier = std::pair<ContentMetaType, ContentType>;
    using ContentEnumerate = std::pair<FsPath, ContentClassifier>;


    constexpr ContentClassifier LogoType{ContentMetaType::Application, ContentType::Control};

#pragma pack(push, 1)
    struct PackagedContentMeta {
        U64 titleid;
        U32 version;
        ContentMetaType type;
        U8 reserved;
        U16 sequencesize;
        U16 contentcount;
        U16 contentmetacount;
        U8 attributes;
        std::array<U8, 0x3> reserved1;
        U32 requiredver;
        U32 reserved2;
    };
#pragma pack(pop)
    // https://switchbrew.org/wiki/CNMT
    class Cnmt {
    public:
        explicit Cnmt(const VfsBackingFilePtr &cnmt);

        ContentMetaType type;
    };
}
