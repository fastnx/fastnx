#include <loaders/nsp_es.h>

namespace FastNx::Loaders {
    NspEs::NspEs(const FsSys::VfsBackingFilePtr &nspf) : AppLoader(nspf, AppType::NspEs) {
    }
}
