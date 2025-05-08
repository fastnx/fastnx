#include <common/async_logger.h>
#include <kernel/kernel.h>
#include <kernel/threads/kscheduler.h>
#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>


namespace FastNx::Kernel::Types {
    void KThread::StartThread() const {
        const auto &scheduler{kernel.scheduler};
        scheduler->SetThreadName(fmt::format("HOS-Thread {}", threadid));

        using namespace std::literals::chrono_literals;
        for (U32 count{}; count < 10; count++)
            scheduler->Sleep(10ms);
        scheduler->Quit();
    }

    void KThread::Initialize(KProcess *process, void *ep, void *_stack, void *_tls) {
        procowner = dynamic_cast<KSynchronizationObject *>(process);
        if (procowner)
            procowner->IncreaseLifetime();

        entrypoint = ep;
        stack = _stack;
        tls = _tls;
    }

    void KThread::Destroyed() {
    }
}
