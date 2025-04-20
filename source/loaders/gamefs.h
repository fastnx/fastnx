#pragma once
#include <loaders/types.h>

#include <loaders/application_directory.h>

namespace FastNx::Loaders {
    class GameFs final : public AppLoader {
    public:
        explicit GameFs(const FsSys::VfsReadOnlyDirectoryPtr &files, bool &isloaded);
        std::vector<U8> GetLogo() override;
        U64 GetTitleId() override;


        std::array<U8, 8> titleid;
    };
}
