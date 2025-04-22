#pragma once
#include <kernel/ksynchronization_object.h>
namespace FastNx::Kernel::Types {
    class KThread : public KSynchronizationObject {
    public:
        explicit KThread(Kernel &_kernel) : KSynchronizationObject(KAutoType::KThread, _kernel) {}

        std::list<KSynchronizationObject *> syncobjs;
    };
}