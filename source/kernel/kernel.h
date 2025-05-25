#pragma once
#include <map>
#include <memory>
#include <mutex>

#include <common/memory.h>
#include <kernel/memory/kslab_heap.h>
#include <kernel/memory/k_memory.h>
#include <kernel/types/kprocess.h>

#include <kernel/types.h>
#include <jit/dynarmic_jit.h>


namespace FastNx::Kernel {
    class Kernel {
    public:
        Kernel();
        U64 GetPid(const ProcessEntropy &entropy);
        U64 GetThreadId();
        std::map<U64, KAutoObject *> autorefs;

        std::shared_ptr<Types::KProcess> CreateProcess();
        std::shared_ptr<Types::KThread> CreateThread();
        void CreateJit(U32 cpunumber);
        std::shared_ptr<Jit::JitDynarmicController> GetJit(U32 corenumber);

        std::shared_ptr<Types::KProcess> GetCurrentProcess();

        std::shared_ptr<NxAllocator> nxalloc;
        std::shared_ptr<Memory::KSlabHeap> poffset;
        std::shared_ptr<Memory::KSlabHeap> userslabs;
        std::shared_ptr<Memory::KMemory> memory;
        std::shared_ptr<Threads::KScheduler> scheduler;

    private:
        U64 pidseed{InitialProcessId};
        U64 threadseed{};
        std::vector<std::pair<U64, const ProcessEntropy *>> pidslist;

        std::list<std::shared_ptr<Types::KProcess>> liveprocs;
        std::mutex pmutex;
    };
}
