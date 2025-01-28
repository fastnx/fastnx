#pragma once

#include <loaders/types.h>
namespace FastNx::Loaders {
    class NspEs : public AppLoader {
    public:
        explicit NspEs(const FsSys::VfsBackingFilePtr &nspf);
    };
}
