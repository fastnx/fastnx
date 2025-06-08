#include <device/memory.h>

#include <common/allocators.h>
namespace FastNx {
    void * Allocate(const U64 size) {
        return Device::AllocateGuestMemory(size, {});
    }
    void Free(void *pointer, const U64 size) {
        Device::FreeMemory(pointer, size);
    }
}
