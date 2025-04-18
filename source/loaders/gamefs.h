#pragma once
#include <loaders/types.h>

#include <loaders/application_directory.h>

namespace FastNx::Loaders {
    class GameFs final : public AppLoader {
    public:
        explicit GameFs(const FsSys::VfsReadOnlyDirectoryPtr &files, bool &isloaded);
        void LoadApplication(std::shared_ptr<Kernel::Types::KProcess> &kprocess) override;
        std::vector<U8> GetLogo() override;
        U64 GetTitleId() override;


        std::shared_ptr<ApplicationDirectory> nextloader;
    };
}
