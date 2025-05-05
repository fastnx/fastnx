#pragma once
#include <loaders/nso_fmt.h>
#include <fs_sys/npdm.h>

namespace FastNx::Horizon {
    class SwitchNs;

    class ProcessLoader {
    public:
        explicit ProcessLoader(const std::shared_ptr<SwitchNs> &nx) : switchnx(nx) {}

        void SetProcessMemory(Kernel::ProcessCodeLayout &&codeset) const;

        void Load();
        std::shared_ptr<Kernel::Types::KProcess> process;
        std::optional<FsSys::Npdm> npdm;

    private:
        static void GetCodeSet(Kernel::ProcessCodeLayout &codeset, const std::shared_ptr<Loaders::NsoFmt> &nso);

        const std::shared_ptr<SwitchNs> switchnx;
    };
}
