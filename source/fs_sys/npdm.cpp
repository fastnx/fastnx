#include <boost/align/is_aligned.hpp>
#include <common/values.h>
#include <common/async_logger.h>
#include <fs_sys/npdm.h>

namespace FastNx::FsSys {
    Npdm::Npdm(const VfsBackingFilePtr &npdm) {
        const auto meta{npdm->Read<Meta>()};
        if (meta.magic != ConstMagicValue<U32>("META"))
            return;
        if (const std::string_view product{meta.productstrs.data(), strlen(meta.productstrs.data())}; !product.empty())
            AsyncLogger::Info("NPDM application name: {}", product);

        if (meta.preallocate > 0x1FE00000)
            return;
        if (!boost::alignment::is_aligned(meta.mainthreadstack, 0x1000))
            return;

        procflags = meta.flags;
        stacksize = meta.mainthreadstack;
    }

}
