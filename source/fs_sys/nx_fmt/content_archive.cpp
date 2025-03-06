#include <common/container.h>
#include <fs_sys/xts_file.h>
#include <fs_sys/nx_fmt/content_archive.h>

namespace FastNx::FsSys::NxFmt {
    auto CheckNcaMagic(const U32 value) {
        static std::vector<U32> magics;
        if (magics.empty()) {
            magics.reserve(3);
            for (const auto &magic : {"NCA0", "NCA1", "NCA2", "NCA3"})
                magics.emplace_back(ConstMagicValue<U32>(magic));
        }
        return Contains(magics, value);
    }
    ContentArchive::ContentArchive(const VfsBackingFilePtr &nca) {
        if (nca->GetSize() < sizeof(NcaHeader))
            return;
        auto _archive{nca->Read<NcaHeader>()};
        if (const auto magic{_archive.magic})
            encrypted = !CheckNcaMagic(magic);

        if (encrypted) {
            _nca = std::make_shared<XtsFile>(std::move(nca), Crypto::SourceKey<256>{});
            _archive = _nca->Read<NcaHeader>();
        } else {
            _nca = std::move(nca);
        }
        type = _archive.type;
    }
}
