#pragma once

#include <kernel/types.h>
#include <kernel/ksynchronization_object.h>
#include <kernel/memory/ktls_pagemanager.h>
#include <kernel/memory/k_memory.h>
#include <kernel/types/kthread.h>

namespace FastNx::Kernel::Types {
    class KProcess final : public KSynchronizationObject {
    public:
        explicit KProcess(Kernel &_kernel);
        void Initialize(U64 stack, const ThreadPriority &thprior, U8 desiredcore);


        std::list<Memory::KTlsPageManager> freetlslist;
        std::list<Memory::KTlsPageManager> fulltlslist;
        U8 *AllocateTls();

        U64 processid{};
        ProcessEntropy entropy{};

        std::shared_ptr<Memory::KMemory> &memory;
        std::span<U8> procmem{};

        std::list<std::shared_ptr<KThread>> threads{};
        bool hasstarted{};

        void *entrypoint{};
        U8 *tlsarea{};
    private:
        void Destroyed() override;
    };
}
