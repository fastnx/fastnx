#include <cstring>
#include <kernel/types/kprocess.h>
#include <kernel/kernel.h>

namespace FastNx::Kernel::Types {
    KProcess::KProcess(Kernel &_kernel) : KSynchronizationObject(KAutoType::KProcess, _kernel) {
        std::memset(entropy.data(), 0, sizeof(entropy));

        processId = kernel.GetPid(*this);
    }

    void KProcess::Destroyed() {
    }
}
