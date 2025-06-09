#pragma once
#include <condition_variable>

#include <jit/types.h>
#include <kernel/ksynchronization_object.h>

namespace FastNx::Kernel::Types {
    class KThread final : public KSynchronizationObject {
    public:
        explicit KThread(Kernel &_kernel) : KSynchronizationObject(KAutoType::KThread, _kernel) {}

        void Initialize(KProcess *process, void *ep, void *_stack, void *_tls, U32 firstcpu);
        void ResumeThread();

        void *entrypoint{};
        void *stack{};
        void *exceptiontls{};
        void *usertls{};
        U64 threadid{};

        U32 desiredcpu{};

        U32 state{};
        std::mutex condmutex;
        std::condition_variable condsched;

    private:
        Jit::JitThreadContext jitload{};

        std::list<KSynchronizationObject *> syncobjs;
        KSynchronizationObject *parent{}; // Process this thread belongs to
    protected:
        void Destroyed() override;
    };
}
