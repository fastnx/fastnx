#pragma once
#include <kernel/ksynchronization_object.h>
namespace FastNx::Kernel::Types {
    class KThread final : public KSynchronizationObject {
    public:
        explicit KThread(Kernel &_kernel) : KSynchronizationObject(KAutoType::KThread, _kernel) {}

        void StartThread() const;
        void Initialize(KProcess *process, void *ep, void *_stack, void *_tls);

        void *entrypoint{};
        void *stack{};
        void *tls{};
        U64 threadid{};
        std::list<KSynchronizationObject *> syncobjs;
        KSynchronizationObject *procowner{}; // Process this thread belongs to

    protected:
        void Destroyed() override;
    };
}