#pragma once

#include <filesystem>
#include <vector>
#include <cassert>
#include <cstring>
#include <utility>

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
        virtual ~VfsBackingFile() = default;

        explicit VfsBackingFile(const FsPath &_path, const AccessModeType _mode = AccessModeType::ReadOnly) : path(_path), mode(_mode) {}

        template<typename T> requires (std::is_trivial_v<T>)
        T Read(const U64 _offset = {}) {
            T value;
            std::memset(&value, 0, sizeof(T));
            ReadType(reinterpret_cast<U8*>(&value), sizeof(T), _offset);
            return value;
        }

        template<typename T> requires (std::is_trivial_v<T>)
        U64 Read(T &object, const U64 _offset = {}) {
            std::memset(&object, 0, sizeof(T));
            return ReadType(reinterpret_cast<U8*>(&object), sizeof(T), _offset);
        }

        FsPath path;
        AccessModeType mode;

    private:
        U64 ReadType(U8 *dest, const U64 size, const U64 offset) {
            if (mode == AccessModeType::WriteOnly)
                throw std::runtime_error("Operation not supported on the file");
            if (!dest && !size)
                return {};
            return ReadTypeImpl(dest, size, offset);
        }

    protected:
        virtual U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) = 0;
    };

    using VfsBackingFilePtr = std::shared_ptr<VfsBackingFile>;

    class VfsReadOnlyDirectory {
    public:
        explicit VfsReadOnlyDirectory(const FsPath &_path) : path(_path) {}

        virtual ~VfsReadOnlyDirectory() = default;
        std::vector<FsPath> BlobAllFiles(const std::string &pattern, bool followTree = {});
        virtual std::vector<FsPath> ListAllFiles() = 0;
        virtual std::vector<FsPath> ListAllTopLevelFiles() const = 0;

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

constexpr FastNx::FsSys::FsPath operator ""_fs(const char *str, const FastNx::U64 len) {
    if (const std::string_view _pathStr{str, len}; !_pathStr.empty())
        return FastNx::FsSys::FsPath{_pathStr};

    std::unreachable();
}
