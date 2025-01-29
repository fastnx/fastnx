#pragma once

#include <filesystem>
#include <vector>

#include <common/types.h>
namespace FastNx::FsSys {
    using FsPath = std::filesystem::path;
    class SolidDirectory;

    enum class AccessModeType {
        ReadOnly,
        WriteOnly,
        ReadWrite,
        None,
    };
    class VfsBackingFile {
    public:
        explicit VfsBackingFile(const FsPath &_path, const AccessModeType _mode = AccessModeType::ReadOnly) : path(_path), mode(_mode) {}
        FsPath path;
        AccessModeType mode;
    };

    using VfsBackingFilePtr = std::shared_ptr<VfsBackingFile>;

    class VfsReadOnlyDirectory {
    public:
        explicit VfsReadOnlyDirectory(const FsPath &_path) : path(_path) {}
        virtual ~VfsReadOnlyDirectory() = default;

        std::vector<FsPath> BlobAllFiles(const std::string &pattern);
        virtual std::vector<FsPath> ListAllFiles() = 0;
        virtual U64 GetFilesCount() {
            return {};
        }
        FsPath path;
    };
    class VfsBackingDirectory : public VfsReadOnlyDirectory {
    public:
        explicit VfsBackingDirectory(const FsPath &_path) : VfsReadOnlyDirectory(_path) {}
    };

    class RegexFile {
    public:
        explicit RegexFile(const FsPath &_path, const std::string &pattern);
        std::vector<std::string> matches;
    };

    bool IsInsideOf(const FsPath &path, const FsPath &is);
}
