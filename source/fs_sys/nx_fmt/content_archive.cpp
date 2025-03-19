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

        _nca = [&] -> VfsBackingFilePtr {
            if (!encrypted)
                return std::move(nca);
            if (!keys->headerKey)
                throw exception{"Header key not found"};
            auto xts = std::make_shared<XtsFile>(std::move(nca), *keys->headerKey.value());
            xts->doublebuf = true;
            archive = xts->Read<NcaHeader>();
            return xts;
        }();

        assert(CheckNcaMagic(archive.magic));
        assert(archive.contentSize == nca->GetSize());

        if (!Crypto::VerifyNcaSignature(&archive.magic, 0x200, archive.signature.header))
            AsyncLogger::Info("Header signature verification failed");

        size = archive.contentSize;
        type = archive.type;
    }
}
