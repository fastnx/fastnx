#include <runtime/entropy.h>
#include <kernel/kernel.h>

#include <kernel/threads/kscheduler.h>
#include <kernel/types/kprocess.h>



namespace FastNx::Kernel::Types {
    KProcess::KProcess(Kernel &_kernel) : KSynchronizationObject(KAutoType::KProcess, _kernel), memory(kernel.memory) {
        Runtime::GetEntropy(entropy);
        processid = kernel.GetPid(entropy);
    }

    void KProcess::Initialize(const U64 stack, [[maybe_unused]] const ThreadPriority &priority, const U8 desiredcore) {
        entrypoint = memory->code.begin().base();
        kernelexcepttls = AllocateTls();
        stacksize = stack;

        auto mainthread{kernel.CreateThread()};
        if (!mainthread)
            return;
        std::scoped_lock lock{threadind};

        memory->MapStackMemory(0, stacksize);
        mainthread->Initialize(this, entrypoint, memory->stack.begin().base() + stacksize, kernelexcepttls, desiredcore);
        const auto firstthread{handletable.Allocate(mainthread)};
        threads.emplace_back(std::move(mainthread));

        NX_ASSERT(handletable.GetObject<KThread>(firstthread));
    }

    void KProcess::Start() const {
        // At this point, we will not return until the process has finished
        if (threads.empty())
            return;
        const auto &scheduler{kernel.scheduler};
        if (!scheduler->ismultithread) {
            bool running{true};
            while (running) {
                auto firstfiber{boost::fibers::fiber(&Threads::KScheduler::Reeschedule, scheduler)};
                if ((running = firstfiber.joinable()))
                    firstfiber.join();
            }
        }
        scheduler->Reeschedule();
    }

    void KProcess::Kill() {
        for (const auto &livethread: threads) {
            kernel.scheduler->KillThread(livethread);
        }
        threads.clear();
    }

    U8 * KProcess::AllocateTls() {
        std::scoped_lock lock{threadind};

        U8 *threadstorage{nullptr};
        if (!freetlslist.empty()) {
            const auto tls{freetlslist.begin()};
            if ((threadstorage = tls->Allocate()) != nullptr)
                if (!tls->Allocatable())
                    fulltlslist.splice(fulltlslist.begin(), freetlslist, tls);
        } else {
            const auto tlsaddr{reinterpret_cast<U64>(kernel.userslabs->Allocate())};
            memory->MapTlsMemory(tlsaddr, SwitchPageSize);
            freetlslist.emplace_back(memory->tlsio.begin().base() + tlsaddr);
            return AllocateTls();
        }

        return threadstorage;
    }

    void KProcess::Destroyed() {
        handletable.CloseAll();
        threads.clear();
    }
}
