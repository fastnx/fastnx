#include <boost/endian/detail/endian_reverse.hpp>

#include <common/container.h>
#include <common/exception.h>
#include <common/values.h>
#include <common/async_logger.h>
#include <horizon/key_set.h>
#include <fs_sys/xts_file.h>
#include <fs_sys/ctr_file.h>
#include <fs_sys/offset_file.h>

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
    ContentArchive::ContentArchive(const VfsBackingFilePtr &nca, const std::shared_ptr<Horizon::KeySet> &ks) : keys(ks), ncavfs(nca) {
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
            const auto xts = std::make_shared<XtsFile>(std::move(nca), *keys->headerKey.value());
            content = xts->Read<NcaHeader>();
            return xts;
        }();

        NX_ASSERT(CheckNcaMagic(content.magic));
        NX_ASSERT(content.size == ncavfs->GetSize());

        if (!Crypto::VerifyNcaSignature(&content.magic, 0x200, content.rsaheader))
            AsyncLogger::Info("Header signature verification failed");

        size = content.size;
        type = content.type;
        titleid = content.programId;

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
            const auto fsheader{ncavfs->Read<FsHeader>(0x400 + fsindex * 0x200)};
            NX_ASSERT(fsheader.version == 2);
            const auto file{GetFile(content.fileentries[fsindex], fsheader, content)};
            if (!file)
                return;

            if (fsheader.type == FsType::PartitionFs)
                pfslist.emplace_back(std::make_shared<PartitionFileSystem>(std::move(file)));
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
            if (const auto filelayer{layers.back()}; !IsZeroes(filelayer)) {
                return FileProperties{filelayer.logical, filelayer.hashdatasz};
            }
        } else {
            const auto &hierarchical{fsheader.hierarchical};
            const auto layer{hierarchical.layercount - 1};
            if (const auto fileregion{hierarchical.regions.at(layer)}; !IsZeroes(fileregion)) {
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
        const auto DecryptAreaKey = [&] (const Horizon::KeyIndexType type) {
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
            const auto kek{*keys->GetIndexableKey(type, generation)};
            Copy(decryptedkey, content.encKeyArea.at(index));

            Crypto::AesCipher ecbdecrypt{ToSpan(kek), Crypto::AesMode::Decryption, Crypto::AesType::AesEcb128};
            ecbdecrypt.Process(decryptedkey.data(), decryptedkey.data(), 16);

            return decryptedkey;
        };

        if (const auto &rights{content.rights}; !IsZeroes(rights)) {
            const auto ticket{keys->GetTicket(rights)};
            if (!ticket)
                throw exception{"Required ticket not available"};
            return (*ticket)->DecryptTitleKey(*keys->GetIndexableKey(Horizon::KeyIndexType::Titlekek, GetKeyGeneration(content)));
        }

        using KeyAreaType = Horizon::KeyIndexType;
        switch (content.keyIndex) {
            case KeyAreaEncryptionKeyIndex::Application:
                return DecryptAreaKey(KeyAreaType::KeyAreaApplication);
            case KeyAreaEncryptionKeyIndex::Ocean:
                return DecryptAreaKey(KeyAreaType::KeyAreaOcean);
            case KeyAreaEncryptionKeyIndex::System:
                return DecryptAreaKey(KeyAreaType::KeyAreaSystem);
        }
        return std::nullopt;
    }

    void GetCtr(std::array<U8, 16> &result, const FsHeader &fsheader) {
        NX_ASSERT(IsZeroes(result));
        *reinterpret_cast<U32 *>(result.data()) = boost::endian::endian_reverse(fsheader.securevalue);
        *reinterpret_cast<U32 *>(result.data() + 4) = boost::endian::endian_reverse(fsheader.generation);
    }

    constexpr auto NcaSectorSize{0x200};

    VfsBackingFilePtr ContentArchive::GetFile(const FsEntry &fileentry, const FsHeader &fsheader, const NcaHeader &content) {
        const auto maxOffset{fileentry.end * NcaSectorSize};
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
            properties->offset += fileentry.start * NcaSectorSize;
            return *properties;
        }();

        if (CalculateCoverage(fileoffset + filesize, maxOffset) < 70)
            AsyncLogger::Info("This NCA {} is split into multiple files", GetPathStr(ncavfs));

        const auto file = [&] -> VfsBackingFilePtr {
            const auto decryptkey{GetDecryptionKey(fsheader, content)};
            if (!decryptkey)
                return nullptr;

            std::array<U8, 16> counter{};
            const auto ncafile{encrypted ? std::dynamic_pointer_cast<XtsFile>(ncavfs)->encfile : ncavfs};

            switch (fsheader.encryptionType) {
                case EncryptionType::AesCtr: [[fallthrough]];
                case EncryptionType::AesCtrEx:
                    GetCtr(counter, fsheader);

                    return std::make_shared<CtrFile>(ncafile, static_cast<const Crypto::Key128 &>(*decryptkey), counter, fileoffset, filesize);
                case EncryptionType::AesXts:
                    return std::make_shared<XtsFile>(ncafile, Crypto::Key256{}, fileoffset, filesize);
                default:
                    return std::make_shared<OffsetFile>(ncafile, ncafile->path, fileoffset, filesize);
            }
        }();

        return files.emplace_back(fsheader.type, file).second;
    }
}
