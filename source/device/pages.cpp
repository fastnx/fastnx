#include <sys/mman.h>
#include <unistd.h>

#include <common/exception.h>
#include <device/capabilities.h>

namespace FastNx::Device {
    U64 GetHostPageSize() {
        static const auto size{sysconf(_SC_PAGESIZE)};
        return static_cast<U64>(size);
    }

    void LockAll() {
        if (mlockall(MCL_FUTURE) != 0)
            if (errno == EPERM)
                throw exception{"The current process does not have the `CAP_IPC_LOCK` privilege"};
    }
}
