#include <cassert>
#include <unordered_map>
#include <fcntl.h>

#include <fs_sys/types.h>
namespace FastNx::FsSys {
    I32 GetIoMode(const AccessModeType type) {
        I32 result{};
        if (type == AccessModeType::ReadOnly)
            result |= O_RDONLY;
        if (type == AccessModeType::ReadWrite)
            result |= O_RDWR;
        if (type == AccessModeType::WriteOnly)
            result |= O_WRONLY;

        return result;
    }

    bool IsInsideOf(const FsPath &path, const FsPath &is) {
        assert(exists(path) && exists(is));
        auto dir{path.begin()};
        for (auto it{is.begin()}; dir != path.end() && it != is.end(); ++dir, ++it) {
            if (*it != *dir)
                return {};
        }
        return true;
    }

    enum class FileFormatType {
        Unknown,
        PartFs
    };
    bool IsFileOfType(const VfsBackingFilePtr &fptr, const FileFormatType type) {
        static const std::unordered_map<U32, FileFormatType> formatmap{
            {ConstMagicValue<U32>("PFS0"), FileFormatType::PartFs}
        };
        const auto uType{fptr->Read<U32>()};
        if (const auto &_typed{formatmap.find(uType)}; _typed != formatmap.end())
            return _typed->second == type;
        return {};
    }
    bool IsAPfs0File(const VfsBackingFilePtr &pfs0) {
        if (IsFileOfType(pfs0, FileFormatType::PartFs))
            return pfs0->Read<U32>(4) == 0;
        return {};
    }
}
