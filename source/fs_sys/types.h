#pragma once

#include <filesystem>
#include <vector>
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
        void Write(T& value, const U64 _offset = {}) {
            WriteType(reinterpret_cast<U8 *>(&value), sizeof(T), _offset);
        }

        template<typename T> requires (std::is_trivial_v<T>)
        U64 Read(T &object, const U64 _offset = {}) {
            std::memset(&object, 0, sizeof(T));
            return ReadType(reinterpret_cast<U8 *>(&object), sizeof(T), _offset);
        }

        template<typename T = U8> requires (!IsVectorType<T>)
        std::vector<T> ReadSome(const U64 size, const U64 _offset) {
            std::vector<T> _content(size);
            NX_ASSERT(ReadTypeImpl(reinterpret_cast<U8*>(_content.data()), size, _offset) == size);
            return _content;
        }
        template<typename T>
        U64 ReadSome(const std::span<T> &content, const U64 _offset) {
            return ReadTypeImpl(reinterpret_cast<U8*>(content.data()), content.size(), _offset);
        }
        template<typename T>
        U64 WriteSome(const std::span<T> &content, const U64 _offset) {
            return WriteTypeImpl(reinterpret_cast<const U8*>(content.data()), content.size(), _offset);
        }
        std::string ReadLine(U64 offset = {});
        std::vector<std::string> GetAllLines();

        virtual explicit operator bool() const = 0;
        virtual U64 GetSize() const = 0;

        FsPath path;
        FileModeType mode;

        template<bool Read>
        U64 PerformSafeIo(auto *data, const U64 size, const U64 offset) {
            const auto priviledge = [&] -> bool {
                if (mode == FileModeType::ReadWrite)
                    return true;

                if (mode == FileModeType::ReadOnly && Read)
                    return true;
                if (mode == FileModeType::WriteOnly && !Read)
                    return true;

                throw std::logic_error("Unsupported file access mode");
            }();
            if (!priviledge)
                return {};

            if (!data && !size)
                return {};
            if constexpr (Read)
                return ReadTypeImpl(data, size, offset);
            else
                return WriteTypeImpl(data, size, offset);
        }
        template<typename T> requires (sizeof(T) == 1)
        U64 ReadType(T *dest, const U64 size, const U64 offset) {
            return PerformSafeIo<true>(reinterpret_cast<U8*>(dest), size, offset);
        }
        template<typename T> requires (sizeof(T) == 1)
        U64 WriteType(const T *source, const U64 size, const U64 offset) {
            return PerformSafeIo<false>(reinterpret_cast<const U8*>(source), size, offset);
        }
    private:
        virtual U64 ReadTypeImpl(U8 *dest, U64 size, U64 offset) = 0;
        virtual U64 WriteTypeImpl(const U8* source, U64 size, U64 offset) = 0;
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

    bool IsInsideOf(const FsPath &path, const FsPath &is, bool check = true);
    bool IsAPfs0File(const VfsBackingFilePtr &pfs0);

    U64 GetSizeBySeek(I32 fd);
    bool MatchFiles(const VfsBackingFilePtr &first, const VfsBackingFilePtr &second);

    template<typename T>
    std::string GetPathStr(const T &value) {
        if constexpr (std::is_same_v<T, FsPath>) {
            return value.string();
        } else if constexpr (std::is_same_v<T, VfsBackingFilePtr>) {
            return GetPathStr(value->path);
        } else {
            [[assume(std::is_same_v<T, VfsBackingDirectory>)]];
            return GetPathStr(value.path);
        }
    }
}

constexpr FastNx::FsSys::FsPath operator ""_fs(const char *str, const FastNx::U64 len) {
    if (const std::string_view _pathStr{str, len}; !_pathStr.empty())
        return FastNx::FsSys::FsPath{_pathStr};

    std::unreachable();
}
