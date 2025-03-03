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

    enum class FileModeType {
        ReadOnly,
        WriteOnly,
        ReadWrite,
        None,
    };
    class VfsBackingFile {
    public:
        virtual ~VfsBackingFile() = default;

        explicit VfsBackingFile(const FsPath &_path, const FileModeType _mode = FileModeType::ReadOnly) : path(_path), mode(_mode) {}

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

        template<typename T = U8> requires (!IsVectorType<T>)
        std::vector<T> ReadSome(const U64 _offset, const U64 size) {
            std::vector<T> _content(size);
            assert(ReadTypeImpl(reinterpret_cast<U8*>(_content.data()), size, _offset) == size);
            return _content;
        }
        template<typename T>
        U64 ReadSome(std::vector<T> &content, const U64 _offset) {
            return ReadTypeImpl(reinterpret_cast<U8*>(content.data()), content.size(), _offset);
        }
        std::string ReadLine(U64 offset = {});
        std::vector<std::string> GetAllLines();

        virtual explicit operator bool() const = 0;
        virtual U64 GetSize() const = 0;

        FsPath path;
        FileModeType mode;

        U64 ReadType(U8 *dest, const U64 size, const U64 offset) {
            if (mode == FileModeType::WriteOnly) {
                throw std::runtime_error{"Operation not supported on the file"};
            }

            if (!dest && !size)
                return {};
            return ReadTypeImpl(dest, size, offset);
        }
    private:
        virtual U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) = 0;
    };

    using VfsBackingFilePtr = std::shared_ptr<VfsBackingFile>;

    class VfsReadOnlyDirectory {
    public:
        explicit VfsReadOnlyDirectory(const FsPath &_path) : path(_path) {}

        virtual ~VfsReadOnlyDirectory() = default;
        std::vector<FsPath> GlobAllFiles(const std::string &pattern, bool followTree = {}) const;

        virtual VfsBackingFilePtr OpenFile(const FsPath &_path, FileModeType mode = FileModeType::ReadOnly) = 0;
        virtual std::vector<FsPath> ListAllFiles() const = 0;
        [[nodiscard]] virtual std::vector<FsPath> ListAllTopLevelFiles() const = 0;

        virtual U64 GetFilesCount() const {
            return {};
        }
        FsPath path;
    };
    using VfsReadOnlyDirectoryPtr = std::shared_ptr<VfsReadOnlyDirectory>;

    class VfsBackingDirectory : public VfsReadOnlyDirectory {
    public:
        explicit VfsBackingDirectory(const FsPath &_path) : VfsReadOnlyDirectory(_path) {}
    };

    I32 ModeToNative(FileModeType type);
    std::optional<FsPath> GetFullPath(const FsPath &_path);

    bool IsInsideOf(const FsPath &path, const FsPath &is);
    bool IsAPfs0File(const VfsBackingFilePtr &pfs0);

    U64 GetSizeBySeek(I32 fd);

    template<typename T>
    std::string GetPathStr(const T &value) {
        if constexpr (std::is_same_v<T, FsPath>) {
            return value.string();
        } else {
            [[assume(std::is_same_v<T, VfsBackingFilePtr>)]];
            return GetPathStr(value->path);
        }
    }
}

constexpr FastNx::FsSys::FsPath operator ""_fs(const char *str, const FastNx::U64 len) {
    if (const std::string_view _pathStr{str, len}; !_pathStr.empty())
        return FastNx::FsSys::FsPath{_pathStr};

    std::unreachable();
}
