#pragma once

#include <filesystem>
#include <vector>
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

    class VfsBackingDirectory {
    public:
        virtual ~VfsBackingDirectory() = default;
        explicit VfsBackingDirectory(const FsPath &_path) : path(_path) {}
        std::vector<FsPath> BlobAllFiles(const std::string& pattern);
        virtual std::vector<FsPath> ListAllFiles() = 0;

        FsPath path;
    };

    class RegexFile {
    public:
        explicit RegexFile(const FsPath &_path, const std::string &pattern);
        std::vector<std::string> matches;
    };

    bool IsInsideOf(const FsPath &path, const FsPath &is);
}
