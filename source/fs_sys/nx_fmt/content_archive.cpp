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
        auto content{nca->Read<NcaHeader>()};
        if (const auto magic{content.magic})
            encrypted = !CheckNcaMagic(magic);

        ncavfs = [&] -> VfsBackingFilePtr {
            if (!encrypted)
                return std::move(nca);
            if (!keys->headerKey)
                throw exception{"Header key not found"};
            auto xts = std::make_shared<XtsFile>(std::move(nca), *keys->headerKey.value());
            content = xts->Read<NcaHeader>();
            return xts;
        }();

        NX_ASSERT(CheckNcaMagic(content.magic));
        NX_ASSERT(content.size == ncavfs->GetSize());

        if (!Crypto::VerifyNcaSignature(&content.magic, 0x200, content.rsaheader))
            AsyncLogger::Info("Header signature verification failed");

        size = content.size;
        type = content.type;

        LoadAllContent(content);
    }

    void ContentArchive::LoadAllContent(const NcaHeader &content) {
        const auto entries = [&] {
            U64 count{};
            for (const auto &entry: content.fileentries) {
                if (const auto notzero{!IsZeroes(entry)})
                    count += notzero;
                else break;
            }
            return count;
        }();

        for (U64 fsindex{}; fsindex < entries; fsindex++) {
            const auto fsinfo{ncavfs->Read<FsHeader>(0x400 + fsindex * 0x200)};
            NX_ASSERT(fsinfo.version == 2);
            if (!GetFile(content.fileentries[fsindex], fsinfo, content))
                return;
        }
    }

    struct FileProperties {
        U64 offset;
        U64 size;
    };
    std::optional<FileProperties> GetFileProperties(const bool isintegrity, const FsHeader &fsheader) {
        const auto &integrity{fsheader.integrity};

        if (const auto lastlayer{integrity.infolevelhash.maxlayers - 2 - 1}; isintegrity && lastlayer) {
            const auto &layers{integrity.infolevelhash.levels};
            NX_ASSERT(lastlayer >= 0 || lastlayer < std::size(layers));
            NX_ASSERT(layers.begin() + lastlayer == std::prev(layers.end() - 1));
            if (const auto filelayer{layers.back()}; !IsZeroes(filelayer)) {
                return FileProperties{filelayer.logical, filelayer.hashdatasz};
            }
        } else {
            const auto &hierarchical{fsheader.hierarchical};
            NX_ASSERT(hierarchical.layercount == 2);
            if (const auto fileregion{hierarchical.regions.front()}; !IsZeroes(fileregion)) {
                return FileProperties{fileregion.region, fileregion.size};
            }
        }
        return std::nullopt;
    }

    U8 GetKeyGeneration(const NcaHeader &content) {
        const auto generation{std::min(content.generation, static_cast<U8>(content.generation - 1))};
        return std::max(std::to_underlying(content.kgo), generation);
    }
    std::optional<std::array<U8, 16>> ContentArchive::GetDecryptionKey(const FsHeader &fsheader, const NcaHeader &content) const {
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
            const auto generation{GetKeyGeneration(content)};
            auto kek{*keys->GetIndexableKey(Horizon::KeyIndexType::Titlekek, generation)};
            Copy(decryptedkey, content.encKeyArea.at(index));

            Crypto::SafeAes ecbdecrypt{ToSpan(kek), Crypto::AesMode::Decryption, Crypto::AesType::AesEcb128};
            ecbdecrypt.Process(decryptedkey.data(), decryptedkey.data(), 16);

            return decryptedkey;
        };
       return DecryptKek(content.keyIndex);
    }

    constexpr auto StorageSectorSize{0x200};
    VfsBackingFilePtr ContentArchive::GetFile(const FsEntry &fileentry, const FsHeader &fsheader, const NcaHeader &content) {
        const auto boundoffset{fileentry.end * StorageSectorSize};
        const auto [fileoffset, filesize] = [&] {
            const bool isintegrity{fsheader.hashType == HashType::HierarchicalIntegrityHash};
            if (isintegrity) {
                if (fsheader.integrity.magic != ConstMagicValue<U32>("IVFC"))
                    throw exception{"Header integrity violated"};
                NX_ASSERT(fsheader.type == FsType::RomFs);
            }
            auto properties{GetFileProperties(isintegrity, fsheader)};

            if (!properties)
                throw exception{"Invalid NCA subfile size"};
            properties->offset += fileentry.start * StorageSectorSize;

            return *properties;
        }();

        NX_ASSERT(CalculateCoverage(fileoffset + filesize, boundoffset) > 90);

        const auto file = [&] -> VfsBackingFilePtr {
            const auto decryptkey{GetDecryptionKey(fsheader, content)};
            if (!decryptkey)
                return nullptr;
            NX_ASSERT(decryptkey->size());
            return nullptr;
        }();
        files.emplace_back(fsheader.type, file);
        return file;
    }
}
