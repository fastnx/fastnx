#include <cstring>

#include <common/types.h>
#if defined(linux)
#define pthread_jit_write_protect_np(n) void(n)
#endif

namespace FastNx::Jit::Analyzer {
    void CopyJitCode(void *dest, const void *src, const U64 size) {
        pthread_jit_write_protect_np(0);
        std::memcpy(dest, src, size);
        pthread_jit_write_protect_np(1);
    }
}
