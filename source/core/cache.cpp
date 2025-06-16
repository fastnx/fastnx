#include <filesystem>

#include <common/exception.h>
#include <fs_sys/refs/directory_file_access.h>
#include <core/application.h>
#include <core/tarball.h>
#include <core/cache.h>



namespace FastNx::Core {
    U64 GetCacheDirectorySize() {
        FsSys::ReFs::DirectoryFileAccess cwdfiles{std::filesystem::current_path()};
        U64 total{};
        for (const auto &files: cwdfiles.ListAllFiles()) {
            total += cwdfiles.OpenFile(files)->GetSize();
        }
        return total;
    }

    void ClearCache(const std::shared_ptr<Application> &application) {
        auto DeleteAllFiles = [](FsSys::ReFs::DirectoryFileAccess &directory) {
            std::error_code err;
            if (!directory.filelist.empty())
                for (const auto &valfile: directory.filelist | std::views::values)
                    if (valfile.use_count() > 1)
                        return;
            try {
                for (const auto &files: directory.ListAllFiles()) {
                    directory.filelist.erase(files);
                    std::filesystem::remove(files, err);
                }
            } catch (...) {
                if (err.value())
                    throw exception{err.message()};
            }
        };
        const auto &assets{application->assets};
        if (assets->logs) {
            DeleteAllFiles(*assets->logs);
        }
        if (assets->tiks) {
            for (const auto &tiks: assets->tiks->ListAllFiles())
                application->keys->RemoveTicket(assets->tiks->OpenFile(tiks));
            DeleteAllFiles(*assets->tiks);
        }
    }

    void ExportCache(const std::shared_ptr<Application> &application) {
        const auto &assets{application->assets};
        FsSys::ReFs::DirectoryFileAccess fastnxdir{std::filesystem::current_path()};

        Tarball tar("fastnx.tar");
        for (const auto &files: assets->directory->ListAllFiles()) {
            tar.Include(assets->directory->OpenFile(files));
        }
        tar.Finish();
    }
}
