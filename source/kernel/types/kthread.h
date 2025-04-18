#pragma once
#include <kernel/ksynchronization_object.h>
namespace FastNx::Kernel::Types {
    class KThread : public KSynchronizationObject {
    public:
        explicit KThread(Kernel &_kernel) : KSynchronizationObject(KObjectType::KThread, _kernel) {}

        std::list<KSynchronizationObject*> syncobjs;
    };
}