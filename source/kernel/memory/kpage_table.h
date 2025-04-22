#pragma once
#include <kernel/types.h>
#include <kernel/svc/types.h>
#include <fs_sys/types.h>


namespace FastNx::Kernel::Memory {
    class KPageTable {
    public:
        explicit KPageTable(Kernel &_kernel) : kernel(_kernel) {}

        void CreateForProcess(const std::shared_ptr<Types::KProcess> &process, const Svc::CreateProcessParameter &proccurr, const ProcessCodeLayout &codeset);
    private:
        std::span<U8> addrspace{};
        std::span<U8> coderegion{};
        std::span<U8> heapregion{};

        std::span<U8> aliasregion{};
        std::span<U8> stackregion{};
        std::span<U8> tlsioregion{};

        std::span<U8> aslrregion{};

        Kernel &kernel;
        U8 *baseas{};
        U64 sizeforeom{};
    };
}
