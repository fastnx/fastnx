
#include <unistd.h>
#include <fs_sys/types.h>
namespace FastNx::FsSys {

    U64 GetSizeBySeek(const I32 fd) {
        const auto _current{lseek(fd, 0, SEEK_CUR)};
        const auto result{lseek64(fd, 0, SEEK_END)};
        lseek64(fd, _current, SEEK_SET);
        return result;
    }
}