#pragma once
#include <fs_sys/types.h>

namespace FastNx::FsSys {
#pragma pack(push, 1)
    struct Meta {
        U32 magic; // Always "META"
        U32 signaturekey;
        U32 reserved;
        U8 flags;
        U8 reserved1;
        U8 mainthread; // Describes the priority of the main thread
        U8 mainthreadcore; // The optimal core for this process. Our KScheduler should prioritize it
        U32 reserved2;
        U32 preallocate;
        U32 version;
        U32 mainthreadstack; // The initial stack size for this process
        std::array<char, 0x50> productstrs;
        U32 acioffset;
        U32 acisize;

        U32 acidoffset;
        U32 acidsize;
    };
#pragma pack(pop)

    class Npdm {
    public:
        explicit Npdm(const VfsBackingFilePtr &npdm);
    };
}
