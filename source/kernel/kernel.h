#pragma once
#include <map>
#include <memory>
#include <mutex>

#include <common/memory.h>
#include <kernel/memory/kslab_heap.h>
#include <kernel/memory/k_memory.h>
#include <kernel/types/kprocess.h>

#include <kernel/types.h>


namespace FastNx::Kernel {
    class Kernel {
    public:
        Kernel();
        U64 GetPid(const ProcessEntropy &processent);
        std::map<U64, KAutoObject *> autorefs;


        std::shared_ptr<Types::KProcess> CreateKProcess();
        std::shared_ptr<NxAllocator> nxalloc;
        std::shared_ptr<Memory::KSlabHeap> poffset;
        std::shared_ptr<Memory::KMemory> memory;

    private:
        U64 pidseed{InitialProcessId};
        std::vector<std::pair<U64, const ProcessEntropy *>> pidslist;

        std::list<std::shared_ptr<Types::KProcess>> liveprocs;

        std::mutex pmutex;
    };
}
