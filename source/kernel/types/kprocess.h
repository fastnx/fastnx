#pragma once

#include <common/types.h>
#include <kernel/ksynchronization_object.h>

namespace FastNx::Kernel::Types {
    class KProcess final : public KSynchronizationObject {
    public:
        explicit KProcess(Kernel &_kernel);

        U64 processId;
        ProcessEntropy entropy;
    private:
        void Destroyed() override;
    };
}
