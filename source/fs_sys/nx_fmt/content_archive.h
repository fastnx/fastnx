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
        Gen100,
        Unused,
        Gen300
    };

    enum class KeyAreaEncryptionKeyIndex : U8 {
        Application,
        Ocean,
        System
    };

    struct FsEntry {
        U32 start;
        U32 end;
        U64 reserved;
    };
    static_assert(IsSizeMatch<FsEntry, 0x10>);

    struct alignas(0x200) NcaHeader {
        Crypto::Rsa2048 rsaheader; // Signature over the data from offset 0x200 to 0x400
        Crypto::Rsa2048 rsanpdm;

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
        std::array<FsEntry, 4> fsent;
        std::array<std::array<U8, 0x20>, 4> listhashes;
        std::array<std::array<U8, 0x10>, 4> encKeyArea;
    };
    static_assert(IsSizeMatch<NcaHeader, 0x400>);

    enum class FsType : U8 {
        RomFs,
        PartitionFs
    };
    struct FsHeader {
        U16 version;
        FsType type;
        U8 hashType;
        U8 encryptionType;
        U8 metadataHashType;
        U16 reserved;
        std::array<U8, 0xF8> hashdata;
        std::array<U8, 0x40> patchinfo;
        U32 generation;
        U32 securevalue;
        std::array<U8, 0x30> sparseinfo;
        std::array<U8, 0x28> compressioninfo;
        std::array<U8, 0x30> hashdatainfo;
        std::array<U8, 0x30> reserved1;
    };
    static_assert(IsSizeMatch<FsHeader, 0x200>);

    class ContentArchive {
    public:
        explicit ContentArchive(const VfsBackingFilePtr &nca, const std::shared_ptr<Horizon::KeySet> &ks);
        void LoadAllContent(const NcaHeader &nch) const;

        bool encrypted{};
        ContentType type;
        U32 version{};
        U64 size{};

        std::shared_ptr<Horizon::KeySet> keys;
        VfsBackingFilePtr ncavfs;
    };
}
