#pragma once
#include <list>
#include <boost/container/small_vector.hpp>

#include <fs_sys/types.h>
#include <common/traits.h>
#include <crypto/types.h>
#include <fs_sys/nx_fmt/partition_filesystem.h>


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
        GenUnused,
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
        U64 size;
        U64 programId;
        U32 contentIndex;
        U32 sdkAddonVersion;
        U8 generation;
        U8 signatureKey;
        std::array<U8, 0xE> reserved;
        Crypto::RightsId rights;
        std::array<FsEntry, 4> fileentries;
        std::array<std::array<U8, 0x20>, 4> listhashes;
        std::array<std::array<U8, 0x10>, 4> encKeyArea;
    };
    static_assert(IsSizeMatch<NcaHeader, 0x400>);

    enum class FsType : U8 {
        RomFs,
        PartitionFs
    };
    enum class HashType : U8 {
        Auto,
        None,
        HierarchicalSha256Hash,
        HierarchicalIntegrityHash, // [14.0.0+]
        AutoSha3,
        HierarchicalSha3256Hash,
        HierarchicalIntegritySha3Hash
    };
    enum class EncryptionType : U8 {
        Auto,
        None,
        AesXts,
        AesCtr,
        AesCtrEx,
        AesCtrSkipLayerHash, // [14.0.0+]
        AesCtrExSkipLayerHash
    };
    enum class MetaDataHashType : U8 {
        None,
        HierarchicalIntegrity
    };

#pragma pack(push, 1)
    struct IntegrityMetaInfo {
        U32 magic; // Magic ("IVFC")
        U32 version;
        U32 masterhashsz;
        struct InfoLevelHash {
            U32 maxlayers;
            struct Level {
                U64 logical;
                U64 hashdatasz;
                U32 blocksize; // (in log2)
                U32 reserved0;
            };

            std::array<Level, 6> levels;
            std::array<U8, 0x20> salt;
        } infolevelhash;
        std::array<U8, 0x20> masterhash;
        std::array<U8, 0x18> reserved1;
    };
    static_assert(IsSizeMatch<IntegrityMetaInfo, 0xF8>);

    struct HierarchicalSha256Data {
        std::array<U8, 0x20> masterhash;
        U32 blocksize;
        U32 layercount; // always 2
        struct Region {
            U64 region;
            U64 size;
        };
        std::array<Region, 5> regions;
        std::array<U8, 0x80> reserved;
    };
    static_assert(IsSizeMatch<HierarchicalSha256Data, 0xF8>);


    struct FsHeader {
        U16 version;
        FsType type;
        HashType hashType;
        EncryptionType encryptionType;
        MetaDataHashType metadataHashType;
        U16 reserved;
        union {
            IntegrityMetaInfo integrity;
            HierarchicalSha256Data hierarchical;
        };
        std::array<U8, 0x40> patchinfo;
        U32 generation;
        U32 securevalue;
        std::array<U8, 0x30> sparseinfo;
        std::array<U8, 0x28> compressioninfo;
        std::array<U8, 0x30> hashdatainfo;
        std::array<U8, 0x30> reserved1;
    };
    static_assert(IsSizeMatch<FsHeader, 0x200>);
#pragma pack(pop)


    class ContentArchive {
    public:
        explicit ContentArchive(const VfsBackingFilePtr &nca, const std::shared_ptr<Horizon::KeySet> &ks);
        void LoadAllContent(const NcaHeader &content);
        VfsBackingFilePtr GetFile(const FsEntry &fileentry, const FsHeader &fsheader, const NcaHeader &content);

        std::optional<std::array<U8, 16>> GetDecryptionKey(const FsHeader &fsheader, const NcaHeader &content) const;

        bool encrypted{};
        ContentType type;
        U32 version{};
        U64 size{};
        U64 titleid{};

        boost::container::small_vector<std::pair<FsType, VfsBackingFilePtr>, 4> files;
        std::list<std::shared_ptr<PartitionFileSystem>> pfslist;

        std::shared_ptr<Horizon::KeySet> keys;
        VfsBackingFilePtr ncavfs;
    };
}
