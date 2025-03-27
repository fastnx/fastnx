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

    void ContentArchive::LoadAllContent(const NcaHeader &nch) const {
        U64 entries{};
        for (const auto &entry: nch.fsent) {
            if (const auto notzero{!IsZeroes(entry)})
                entries += notzero;
            else break;
        }

        std::vector<std::pair<FsEntry, FsHeader>> fshdrs;
        for (U64 fsindex{}; fsindex < entries; fsindex++) {
            fshdrs.emplace_back(nch.fsent[fsindex], ncavfs->Read<FsHeader>(0x400 + fsindex * 0x200));
        }
    }
}
