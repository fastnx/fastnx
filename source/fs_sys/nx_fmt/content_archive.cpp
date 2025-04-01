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

    void ContentArchive::LoadAllContent(const NcaHeader &nch) {
        const auto entries = [&] {
            U64 count{};
            for (const auto &entry: nch.fsent) {
                if (const auto notzero{!IsZeroes(entry)})
                    count += notzero;
                else break;
            }
            return count;
        }();

        for (U64 fsindex{}; fsindex < entries; fsindex++) {
            const auto fsinfo{ncavfs->Read<FsHeader>(0x400 + fsindex * 0x200)};
            NX_ASSERT(fsinfo.version == 2);
            if (!GetFile(nch.fsent[fsindex], fsinfo))
                return;
        }
    }

    struct NcaFileDimensions {
        U64 offset;
        U64 size;
        U64 ends;
    };
    std::optional<NcaFileDimensions> GetFileDimensions(const bool isintegrity, const FsHeader &fsheader) {
        const auto &integrity{fsheader.integrity};
        const auto lastlayer{integrity.infolevelhash.maxlayers - 2 - 1};

        std::optional<NcaFileDimensions> fileinfo;
        if (isintegrity) {
            const auto &layers{integrity.infolevelhash.levels};
            NX_ASSERT(lastlayer >= 0 || lastlayer < std::size(layers));
            NX_ASSERT(layers.begin() + lastlayer == std::prev(layers.end() - 1));

            fileinfo.emplace(layers.back().logical, layers.back().hashdatasz);
        }
        if (fileinfo)
            fileinfo->ends = fileinfo->offset + fileinfo->size;
        return fileinfo;
    }

    VfsBackingFilePtr ContentArchive::GetFile(const FsEntry &fscursor, const FsHeader &fsheader) {
        const auto isHashFs = [&] {
            const bool integrityfs{fsheader.hashType == HashType::HierarchicalIntegrityHash};
            if (integrityfs)
                if (fsheader.integrity.magic != ConstMagicValue<U32>("IVFC"))
                    throw exception{"Header integrity violated"};
            return integrityfs;
        }();

        if (isHashFs)
            NX_ASSERT(fsheader.type == FsType::RomFs);
        const NcaFileDimensions fileinfo = [&] {
            if (const auto result{GetFileDimensions(isHashFs, fsheader)})
                if (result->size && result->offset)
                    return *result;
            throw exception{"Invalid NCA subfile size"};
        }();
        NX_ASSERT(fscursor.end * 0x200 > fileinfo.ends);

        const auto file = [&] -> VfsBackingFilePtr {
            files.emplace_back(fsheader.type, nullptr);
            return nullptr;
        }();
        return file;
    }
}
