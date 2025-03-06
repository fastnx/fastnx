#pragma once
#include <fs_sys/types.h>

#include <common/traits.h>
#include <crypto/types.h>

namespace FastNx::FsSys::NxFmt {
    // https://switchbrew.org/wiki/NCA
    enum class DistributionType : U8 {
        Download, // NSP
        GameCard // XCI
    };
    enum class ContentType : U8 {
        Program,
        Meta,
        Control,
        Manual,
        Data,
        PublicData
    };
    enum class KeyGenerationOld : U8 {
        Gen_100,
        Unused,
        Gen_300
    };

    enum class KeyAreaEncryptionKeyIndex : U8 {
        Application,
        Ocean,
        System
    };

#pragma pack(push, 1)
    struct NcaHeader {
        Crypto::Rsa2048 headerSignature; // Signature over the data from offset 0x200 to 0x400
        Crypto::Rsa2048 npdmSignature;
        // The same, but only valid if using the key found in the NPDM, 0 if the NCA is not of type Application
        U32 magic; // "NCA2", "NCA1" or "NCA0" for pre-1.0.0 NCAs
        DistributionType distribution;
        ContentType type;
        KeyGenerationOld kgo;
        KeyAreaEncryptionKeyIndex keyIndex;
        U64 contentSize;
        U64 programId;
        U32 contentIndex;
        U32 sdkAddonVersion;
        U8 generation;
        U8 signatureKey;
        std::array<U8, 0xE> reserved;
        Crypto::RightsId rights;
    };
#pragma pack(pop)
    static_assert(IsSizeMatch<NcaHeader, 0x240>);

    class ContentArchive {
    public:
        explicit ContentArchive(const VfsBackingFilePtr &nca);

        bool encrypted{};
        ContentType type;
        U32 version{};

        VfsBackingFilePtr _nca;
    };
}
