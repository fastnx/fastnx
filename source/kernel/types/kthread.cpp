#include <common/async_logger.h>
#include <kernel/kernel.h>
#include <kernel/threads/kscheduler.h>
#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>


namespace FastNx::Kernel::Types {
    using namespace std::literals::chrono_literals;
    void KThread::ResumeThread() {
        const auto &scheduler{kernel.scheduler};

        U64 count{};
        for (; state; count++) {
            scheduler->SetThreadName(fmt::format("HOS-Thread {:02}", threadid));
            // Simulating some work in this thread
            scheduler->Sleep(100ms);

            scheduler->Reeschedule();
        }
    }

    void KThread::Initialize(KProcess *process, void *ep, void *_stack, void *_tls) {
        if (process)
            if ((parent = dynamic_cast<KSynchronizationObject *>(process)))
                parent->IncreaseLifetime();

        threadid = kernel.GetThreadId();
        entrypoint = ep;
        stack = _stack;
        exceptiontls = _tls;

        if (!process)
            return;
        // https://switchbrew.org/wiki/Thread_Local_Region
        usertls = process->AllocateTls();

        state++;
    }

    void KThread::Destroyed() {
        if (!parent)
            return;

        const auto process{dynamic_cast<KProcess *>(parent)};
        auto threadsit{process->threads.begin()};
        for (; threadsit != process->threads.end(); ++threadsit) {
            NX_ASSERT((*threadsit)->threadid != threadid);
        }
    }
}
