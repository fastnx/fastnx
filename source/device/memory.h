#pragma once

#include <common/types.h>

namespace FastNx::Device {
    void *AllocateMemory(U64 &size, void *fixed = {}, I32 file = -1, bool enbhuge = {}, bool writable = {});

    void FreeMemory(void *allocated, U64 size);

    U64 GetMemorySize(const void *allocated);
}
