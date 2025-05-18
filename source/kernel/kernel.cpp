#include <common/exception.h>
#include <kernel/threads/kscheduler.h>
#include <kernel/kernel.h>



namespace FastNx::Kernel {
    Kernel::Kernel() :
            nxalloc(std::make_shared<NxAllocator>()),
            memory(std::make_unique<Memory::KMemory>(*this)),
            scheduler(std::make_shared<Threads::KScheduler>(*this)) {

        const auto lastpid{pidseed + MaximumProcessIds};
        pidslist.reserve(lastpid - pidseed + 2);
        for (const auto pidval: std::views::iota(pidseed - 1, lastpid + 1)) {
            pidslist.emplace_back(pidval, nullptr);
        }
        NX_ASSERT(std::prev(pidslist.end())->first == lastpid);

        poffset = std::make_shared<Memory::KSlabHeap>(std::span{static_cast<U8 *>(nullptr), SwitchMemorySize}, SwitchPageSize);
        userslabs = std::make_shared<Memory::KSlabHeap>(std::span{reinterpret_cast<U8 *>(SlabHeap::BaseAddress), SlabHeap::Size}, SwitchPageSize);
    }

    U64 Kernel::GetPid(const ProcessEntropy &entropy) {
        std::scoped_lock lock{pmutex};

        for (auto itPid{std::begin(pidslist)}; itPid != pidslist.end(); ++itPid) {
            if (!itPid->second)
                itPid->second = &entropy;
            if (itPid->second)
                if (*itPid->second == entropy)
                    return itPid->first;
        }
        throw exception{"Could not allocate a PID for the process"};
    }

    U64 Kernel::GetThreadId() {
        std::scoped_lock lock{pmutex};
        return threadseed++;
    }
    std::shared_ptr<Types::KThread> Kernel::CreateThread() {
        const auto thread{std::make_shared<Types::KThread>(*this)};
        if (thread)
            scheduler->Emplace(thread);
        return thread;
    }

    static std::map<U32, std::shared_ptr<Jit::JitDynarmicJitController>> jits;
    void Kernel::CreateJit(U32 cpunumber) {
        if (jits.contains(cpunumber))
            return;

        if (const auto &process{GetCurrentProcess()})
            jits.emplace(cpunumber, std::make_shared<Jit::JitDynarmicJitController>(process->memory));
    }

    // ReSharper disable once CppMemberFunctionMayBeStatic
    std::shared_ptr<Jit::JitDynarmicJitController> Kernel::GetJit(const U32 corenumber) {
        if (jits.contains(corenumber))
            return jits[corenumber];
        return nullptr;
    }

    std::shared_ptr<Types::KProcess> Kernel::GetCurrentProcess() {
        if (liveprocs.empty())
            return nullptr;
        return liveprocs.back();
    }

    std::shared_ptr<Types::KProcess> Kernel::CreateProcess() {
        const auto process{std::make_shared<Types::KProcess>(*this)};
        if (process)
            liveprocs.emplace_back(process);
        return process;
    }

}
