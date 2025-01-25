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

    class RegexFile : VfsBackingFile {
    public:
        explicit RegexFile(const FsPath &_path, const std::string &pattern);

        std::vector<std::string> matches;
    };
    using VfsBackingFilePtr = std::shared_ptr<VfsBackingFile>;
    bool IsInsideOf(const FsPath &path, const FsPath &is);
}
