// ReSharper disable CppUseStructuredBinding
#include <common/async_logger.h>
#include <fs_sys/nacp.h>

namespace FastNx::FsSys {
    Nacp::Nacp(const VfsBackingFilePtr &nacp) {
        if (nacp->GetSize() < sizeof(NacpHeader))
            return;

        const auto languages{nacp->Read<U32>(0x302C)};
        if (std::countr_one(languages) < 0)
            return;

        const auto content{nacp->Read<NacpHeader>()};
        const auto &english{content.apptitles.front()};

        if (const std::string_view title{english.title.data(), strlen(english.title.data())}; !title.empty())
            AsyncLogger::Info("Title about to be loaded: {}", title);
        if (const std::string_view publisher{english.publisher.data(), strlen(english.publisher.data())}; !publisher.empty())
            AsyncLogger::Info("Publishing organization of the content: {}", publisher);

    }

}
