#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>


namespace FastNx::Kernel::Types {
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
