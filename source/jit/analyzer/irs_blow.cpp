#include <cstring>

#if defined(linux)
#define pthread_jit_write_protect_np(n) void(n)
#endif

namespace FastNx::Jit::Analyzer {
    void CopyJitCode(void *dest, const void *src, const size_t size) {
        pthread_jit_write_protect_np(0);
        std::memcpy(dest, src, size);
        pthread_jit_write_protect_np(1);
    }
}
