#include <kernel/kernel.h>
#include <kernel/types/kprocess.h>

#include <runtime/entropy.h>
namespace FastNx::Kernel::Types {
    KProcess::KProcess(Kernel &_kernel) : KSynchronizationObject(KAutoType::KProcess, _kernel), memory(kernel.memory) {
        Runtime::GetEntropy(entropy);
        processid = kernel.GetPid(entropy);
    }

    void KProcess::Destroyed() {
    }
}
