#include <common/container.h>
#include <common/exception.h>
#include <common/async_logger.h>
#include <horizon/key_set.h>
#include <fs_sys/xts_file.h>

#include <crypto/types.h>
#include <fs_sys/nx_fmt/content_archive.h>

namespace FastNx::FsSys::NxFmt {
    auto CheckNcaMagic(const U32 value) {
        static std::vector<U32> magics;
        if (magics.empty()) {
            magics.reserve(3);
            for (const auto &magic: {"NCA0", "NCA1", "NCA2", "NCA3"})
                magics.emplace_back(ConstMagicValue<U32>(magic));
        }
        return Contains(magics, value);
    }
    ContentArchive::ContentArchive(const VfsBackingFilePtr &nca, const std::shared_ptr<Horizon::KeySet> &ks) : keys(ks) {
        if (nca->GetSize() < sizeof(NcaHeader))
            return;
        auto archive{nca->Read<NcaHeader>()};
        if (const auto magic{archive.magic})
            encrypted = !CheckNcaMagic(magic);

        ncavfs = [&] -> VfsBackingFilePtr {
            if (!encrypted)
                return std::move(nca);
            if (!keys->headerKey)
                throw exception{"Header key not found"};
            auto xts = std::make_shared<XtsFile>(std::move(nca), *keys->headerKey.value());
            archive = xts->Read<NcaHeader>();
            return xts;
        }();

        NX_ASSERT(CheckNcaMagic(archive.magic));
        NX_ASSERT(archive.contentSize == ncavfs->GetSize());

        if (!Crypto::VerifyNcaSignature(&archive.magic, 0x200, archive.rsaheader))
            AsyncLogger::Info("Header signature verification failed");

        size = archive.contentSize;
        type = archive.type;

        LoadAllContent(archive);
    }

    void ContentArchive::LoadAllContent(const NcaHeader &archive) {
        const auto entries = [&] {
            U64 count{};
            for (const auto &entry: archive.fsent) {
                if (const auto notzero{!IsZeroes(entry)})
                    count += notzero;
                else break;
            }
            return count;
        }();

        for (U64 fsindex{}; fsindex < entries; fsindex++) {
            const auto fsinfo{ncavfs->Read<FsHeader>(0x400 + fsindex * 0x200)};
            NX_ASSERT(fsinfo.version == 2);
            if (!GetFile(archive.fsent[fsindex], fsinfo, archive))
                return;
        }
    }

    struct FileProperties {
        U64 offset;
        U64 size;
        U64 ends;
    };
    std::optional<FileProperties> GetFileProperties(const bool isintegrity, const FsHeader &fsheader) {
        const auto &integrity{fsheader.integrity};
        const auto lastlayer{integrity.infolevelhash.maxlayers - 2 - 1};
        std::optional<FileProperties> fileinfo;
        if (isintegrity && lastlayer) {
            const auto &layers{integrity.infolevelhash.levels};
            if (const auto filelayer{layers.back()}; !IsZeroes(filelayer))
                fileinfo.emplace(filelayer.logical, filelayer.hashdatasz);

            NX_ASSERT(lastlayer >= 0 || lastlayer < std::size(layers));
            NX_ASSERT(layers.begin() + lastlayer == std::prev(layers.end() - 1));
        } else {
            const auto &hierarchical{fsheader.hierarchical};
            NX_ASSERT(hierarchical.layercount == 2);
            if (const auto fileregion{hierarchical.regions.front()}; !IsZeroes(fileregion)) {
                fileinfo.emplace(fileregion.region, fileregion.size);
            }
        }
        if (fileinfo)
            fileinfo->ends = fileinfo->offset + fileinfo->size;
        return fileinfo;
    }

    U8 GetKeyGeneration(const NcaHeader &archive) {
        auto generation{archive.generation};
        generation = std::min(generation, static_cast<U8>(generation - 1));
        return std::max(std::to_underlying(archive.kgo), generation);
    }
    std::optional<std::array<U8, 16>> ContentArchive::GetDecryptionKey(const FsHeader &fsheader, const NcaHeader &archive) const {
        const auto DecryptKek = [&] ([[maybe_unused]] const KeyAreaEncryptionKeyIndex type) {
            std::array<U8, 16> decryptedkey{};

            const auto index = [&] {
                switch (fsheader.encryptionType) {
                    case EncryptionType::AesCtr:
                    case EncryptionType::AesCtrEx:
                    case EncryptionType::AesXts:
                        return 2;
                    default:
                        return 0;
                }
            }();
            const auto generation{GetKeyGeneration(archive)};
            auto kek{*keys->GetIndexableKey(Horizon::KeyIndexType::Titlekek, generation)};
            Copy(decryptedkey, archive.encKeyArea.at(index));

            Crypto::SafeAes ecbdecrypt{ToSpan(kek), Crypto::AesMode::Decryption, Crypto::AesType::AesEcb128};
            ecbdecrypt.Process(decryptedkey.data(), decryptedkey.data(), 16);

            return decryptedkey;
        };
       return DecryptKek(archive.keyIndex);
    }

    VfsBackingFilePtr ContentArchive::GetFile(const FsEntry &fscursor, const FsHeader &fsheader, const NcaHeader &archive) {
        const FileProperties fileinfo = [&] {
            const bool isintegrity{fsheader.hashType == HashType::HierarchicalIntegrityHash};
            if (isintegrity) {
                if (fsheader.integrity.magic != ConstMagicValue<U32>("IVFC"))
                    throw exception{"Header integrity violated"};
                NX_ASSERT(fsheader.type == FsType::RomFs);
            }
            if (const auto properties{GetFileProperties(isintegrity, fsheader)})
                return *properties;
            throw exception{"Invalid NCA subfile size"};
        }();

        NX_ASSERT(fscursor.end * 0x200 > fileinfo.ends);
        const auto file = [&] -> VfsBackingFilePtr {
            const auto decryptkey{GetDecryptionKey(fsheader, archive)};
            if (!decryptkey)
                return nullptr;
            NX_ASSERT(decryptkey->size());
            return nullptr;
        }();
        files.emplace_back(fsheader.type, file);
        return file;
    }
}
