#pragma once
#include <map>
#include <memory>
#include <mutex>

#include <kernel/memory/device_lpddr4.h>
#include <kernel/memory/kslab_heap.h>
#include <kernel/memory/kpage_table.h>
#include <kernel/types/kprocess.h>

#include <kernel/types.h>


namespace FastNx::Kernel {
    class Kernel {
    public:
        Kernel();
        U64 GetPid(const Types::KProcess &process);
        std::map<U64, KAutoObject*> autorefs;
        std::optional<Memory::KPageTable> tables;

        std::shared_ptr<Types::KProcess> CreateProcess();
    private:
        U64 pidseed{InitialProcessId};
        std::map<U64, std::array<U64, 4>> pidslist;

        std::optional<Memory::KSlabHeap> userslabs;
        std::unique_ptr<Memory::DeviceLppd4> virtmem;

        std::list<std::shared_ptr<Types::KProcess>> liveprocs;

        std::mutex idsMutex;
    };
}
