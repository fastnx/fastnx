#include <fs_sys/cnmt.h>

namespace FastNx::FsSys {
#pragma pack(push, 1)
    struct ApplicationMetaExtendedHeader {
        U64 patchid;
        U32 requiredsysver;
        U32 requiredappver;
    };
    struct PackagedContentInfo {
        std::array<U8, 0x20> hashcnt;
        std::array<U8, 0x10> contentid;
        std::array<U8, 0x6> size;
        ContentType type;
        U8 idoffset;
    };

    struct ContentMetaInfo {
        U8 id;
        U32 version;
        U8 metatype;
        U8 attributes;
        U16 reserved;
    };
#pragma pack(pop)
    Cnmt::Cnmt(const VfsBackingFilePtr &cnmt) {
        const auto metainfo{cnmt->Read<PackagedContentMeta>()};
        U64 offset{sizeof(metainfo)};
        type = metainfo.type;

        if (type == ContentMetaType::Application) {
            const auto appmeta{cnmt->Read<ApplicationMetaExtendedHeader>(offset)};
            NX_ASSERT(appmeta.patchid);
            offset += sizeof(appmeta);
        }
        std::vector<PackagedContentInfo> cntinfos;
        cntinfos.reserve(metainfo.contentcount);
        for (U64 count{}; count < metainfo.contentcount; count++) {
            cntinfos.emplace_back(cnmt->Read<PackagedContentInfo>(offset));
            offset += sizeof(PackagedContentInfo);
        }
    }
}
