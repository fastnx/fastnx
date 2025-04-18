#include <loaders/gamefs.h>

FastNx::Loaders::GameFs::GameFs(const FsSys::VfsReadOnlyDirectoryPtr &files, bool &isloaded) : AppLoader(nullptr, isloaded, AppType::GameFilesystem) {

    std::vector<FsSys::ContentEnumerate> metafiles;
    if (!files->OpenFile("autofiles.txt"))
        return;

    nextloader = std::make_shared<ApplicationDirectory>(files);
}

void FastNx::Loaders::GameFs::LoadApplication() {

}

std::vector<FastNx::U8> FastNx::Loaders::GameFs::GetLogo() {
    return {};
}
FastNx::U64 FastNx::Loaders::GameFs::GetTitleId() {
    return {};
}
