#pragma once
#include <map>
#include <runtime/spin_lock.h>
#include <fs_sys/types.h>


namespace FastNx::FsSys::ReFs {
    class EditableDirectory final : public VfsBackingDirectory {
    public:
        explicit EditableDirectory(const FsPath &_path, bool create = {});
        ~EditableDirectory() override;

        std::vector<FsPath> ListAllFiles() const override;
        std::vector<FsPath> ListAllTopLevelFiles() const override;
        U64 GetFilesCount() const override;

        explicit operator bool() const;

        std::map<FsPath, VfsBackingFilePtr> filelist;
        Runtime::SpinLock spinlock;

        VfsBackingFilePtr OpenFile(const FsPath &_path, FileModeType mode = FileModeType::ReadOnly) override;
        I32 descriptor;
    };
}
