#pragma once
#include <list>
#include <memory>

#include <kernel/types.h>
#include <kernel/kauto_object.h>
namespace FastNx::Kernel {
    class KSynchronizationObject : public KAutoObject {
    public:
        KSynchronizationObject(const KAutoType _type, Kernel &_kernel)
            : KAutoObject(_type, _kernel) {}

        void Signal() const;

        std::list<std::shared_ptr<Types::KThread>> thrSynclist; // List of threads to be signaled upon any event on these objects
    };
}