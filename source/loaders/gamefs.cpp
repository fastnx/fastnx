#include <loaders/gamefs.h>

namespace FastNx::Loaders {
    GameFs::GameFs(const FsSys::VfsReadOnlyDirectoryPtr &files, bool &isloaded) : AppLoader(nullptr, isloaded, AppType::GameFilesystem) {
        std::vector<FsSys::ContentEnumerate> metafiles;
        if (!files->OpenFile("autofiles.txt"))
            return;

        nextloader = std::make_shared<ApplicationDirectory>(files);
    }

    void GameFs::LoadApplication([[maybe_unused]] std::shared_ptr<Kernel::Types::KProcess> &kprocess) {
    }
    std::vector<U8> GameFs::GetLogo() {
        return {};
    }
    U64 GameFs::GetTitleId() {
        return {};
    }
}
