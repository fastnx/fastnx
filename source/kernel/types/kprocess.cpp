#include <runtime/entropy.h>
#include <kernel/kernel.h>
#include <kernel/types/kprocess.h>


namespace FastNx::Kernel::Types {
    KProcess::KProcess(Kernel &_kernel) : KSynchronizationObject(KAutoType::KProcess, _kernel), memory(kernel.memory) {
        Runtime::GetEntropy(entropy);
        processid = kernel.GetPid(entropy);
    }

    void KProcess::Initialize([[maybe_unused]] U64 stack, [[maybe_unused]] const ThreadPriority &thprior, [[maybe_unused]] U8 desiredcore) {
        entrypoint = memory->code.begin().base();

        tlsarea = AllocateTls();

        auto mainthread{kernel.CreateThread()};
        if (!mainthread)
            return;
        mainthread->Initialize(this, entrypoint, nullptr, tlsarea);
        threads.emplace_back(std::move(mainthread));
    }

    U8 * KProcess::AllocateTls() {
        U8 *threadstorage{nullptr};
        if (!freetlslist.empty()) {
            const auto tls{freetlslist.begin()};
            if ((threadstorage = tls->Allocate()) != nullptr)
                if (!tls->Allocatable())
                    fulltlslist.splice(fulltlslist.begin(), freetlslist, tls);
        } else {
            const auto tlsaddr{reinterpret_cast<U64>(kernel.userslabs->Allocate())};
            memory->MapTlsMemory(tlsaddr, SwitchPageSize);
            freetlslist.emplace_back(memory->tlsio.begin().base() + tlsaddr);
            return AllocateTls();
        }

        return threadstorage;
    }

    void KProcess::Destroyed() {
    }
}
