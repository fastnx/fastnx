#pragma once

#include <filesystem>
#include <vector>
#include <cassert>
#include <cstring>
#include <utility>

#include <common/types.h>
#include <common/traits.h>
#include <common/values.h>

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
            ReadType(reinterpret_cast<U8 *>(&value), sizeof(T), _offset);
            return value;
        }

        template<typename T> requires (std::is_trivial_v<T>)
        U64 Read(T &object, const U64 _offset = {}) {
            std::memset(&object, 0, sizeof(T));
            return ReadType(reinterpret_cast<U8 *>(&object), sizeof(T), _offset);
        }

        template<typename T = U8> requires (!is_vector_v<T>)
        std::vector<T> ReadSome(const U64 _offset, const U64 size) {
            std::vector<T> _content(size);
            assert(ReadTypeImpl(reinterpret_cast<U8*>(_content.data()), size, _offset) == size);
            return _content;
        }
        virtual U64 GetSize() const = 0;

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
        std::vector<FsPath> GlobAllFiles(const std::string &pattern, bool followTree = {});
        virtual std::vector<FsPath> ListAllFiles() = 0;
        [[nodiscard]] virtual std::vector<FsPath> ListAllTopLevelFiles() const = 0;

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


    I32 ModeToNative(AccessModeType type);
    std::optional<FsPath> GetFullPath(const FsPath &_path);

    bool IsInsideOf(const FsPath &path, const FsPath &is);
    bool IsAPfs0File(const VfsBackingFilePtr &pfs0);

    U64 GetSizeBySeek(I32 fd);
}

constexpr FastNx::FsSys::FsPath operator ""_fs(const char *str, const FastNx::U64 len) {
    if (const std::string_view _pathStr{str, len}; !_pathStr.empty())
        return FastNx::FsSys::FsPath{_pathStr};

    std::unreachable();
}
