#pragma once

#include <common/types.h>
#include <kernel/ksynchronization_object.h>

namespace FastNx::Kernel::Types {
    class KProcess final : public KSynchronizationObject {
    public:
        explicit KProcess(Kernel &_kernel);

        U64 processid;
        ProcessEntropy entropy;

        Memory::KMemory &memory;
        std::span<U8> procmem;
    private:
        void Destroyed() override;
    };
}
