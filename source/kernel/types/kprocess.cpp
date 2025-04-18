#include <runtime/entropy.h>
#include <kernel/kernel.h>
#include <kernel/types/kprocess.h>

namespace FastNx::Kernel::Types {
    KProcess::KProcess(Kernel &_kernel) : KSynchronizationObject(KAutoType::KProcess, _kernel) {
        Runtime::GetEntropy(std::span{reinterpret_cast<U8* >(entropy.data()), entropy.size() * sizeof(U64)});
        processId = kernel.GetPid(*this);
    }

    void KProcess::Destroyed() {
    }
}
