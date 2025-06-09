#include <common/async_logger.h>
#include <kernel/kernel.h>
#include <kernel/threads/kscheduler.h>
#include <kernel/types/kprocess.h>
#include <kernel/types/kthread.h>


namespace FastNx::Kernel::Types {
    using namespace std::literals::chrono_literals;
    void KThread::ResumeThread() {
        const auto &scheduler{kernel.scheduler};

        scheduler->SetThreadName(fmt::format("HOS-Thread {}", threadid));
        U64 count{};
        for (; state && count < 100; count++) {
            // Simulating some work in this thread
            if (const auto &jit{kernel.GetJit(desiredcpu)}) {
                if (!jit->initialized) {
                    jit->Initialize(exceptiontls, usertls);
                }
                jit->Run(jitload);
            }
            scheduler->Yield();
        }
    }

    void KThread::Initialize(KProcess *process, void *ep, void *_stack, void *_tls, const U32 firstcpu) {
        if (parent = dynamic_cast<KSynchronizationObject *>(process); parent)
            parent->IncreaseLifetime();

        desiredcpu = firstcpu;
        threadid = kernel.GetThreadId();
        entrypoint = ep;
        stack = _stack;
        exceptiontls = _tls;

        // Sets up the JIT parameters
        jitload.stack = static_cast<U8 *>(stack) - 0x1000;
        jitload.pc_counter = entrypoint;
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
