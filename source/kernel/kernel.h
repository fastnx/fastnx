#pragma once
#include <map>
#include <memory>
#include <mutex>

#include <kernel/memory/device_lpddr4.h>
#include <kernel/memory/kslab_heap.h>
#include <kernel/types/kprocess.h>

#include <kernel/types.h>
namespace FastNx::Kernel {
    class Kernel {
    public:
        Kernel();
        I32 GetPid(const Types::KProcess &process);
        std::map<U64, KAutoObject*> autorefs;
    private:
        I32 pidseed{InitialProcessId};
        std::map<I32, std::array<U64, 4>> pidslist;

        std::optional<Memory::KSlabHeap> userslabs;
        std::unique_ptr<Memory::DeviceLppd4> virtmem;

        std::mutex idsMutex;
    };
}
