#include <common/bytes.h>
#include <loaders/gamefs.h>

namespace FastNx::Loaders {
    GameFs::GameFs(const FsSys::VfsReadOnlyDirectoryPtr &files, bool &isloaded) : AppLoader(nullptr, isloaded, AppType::GameFilesystem) {
        std::vector<FsSys::ContentEnumerate> metafiles;
        if (!files->OpenFile("autofiles.txt"))
            return;

        titleid = ToArrayOfBytes<8>(FsSys::GetPathStr(files->path), true);
        appdir = std::make_shared<ApplicationDirectory>(files);
    }

    std::vector<U8> GameFs::GetLogo() {
        if (const auto logo{appdir->GetExefs()->OpenFile("exefs/icon_AmericanEnglish.dat")})
            return logo->ReadSome(logo->GetSize());
        return {};
    }
    U64 GameFs::GetTitleId() {
        U64 integral;
        std::memcpy(&integral, titleid.data(), sizeof(titleid));
        return integral;
    }
}
