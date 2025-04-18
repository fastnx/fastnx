#include <fs_sys/npdm.h>
#include <kernel/memory/kpage_table.h>


namespace FastNx::Kernel::Memory {
    void KPageTable::CreateProcessMemory(const FsSys::VfsBackingFilePtr &npdm) {
        const FsSys::Npdm exenpdm(npdm);
        currpas = exenpdm.procflags.pas;
    }
}
