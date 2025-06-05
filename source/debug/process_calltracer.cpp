#include <cxxabi.h>
#include <common/values.h>
#include <common/exception.h>

#include <debug/process_calltracer.h>

namespace FastNx::Debug {
    ProcessCalltracer::ProcessCalltracer(const std::shared_ptr<Kernel::Types::KProcess> &process) {
        const auto &memory{process->memory};
        U64 basetext{};
        while (true) {
            const auto *texthost{memory->code.begin().base() + basetext};
            const auto textinfo{memory->QueryMemory(texthost)};
            if (!textinfo->base || !Kernel::Memory::MemoryState{textinfo->type}.code)
                break;

            if (textinfo->permission == Kernel::Permission::Text)
                GetMod0(texthost);

            basetext += textinfo->size;
        }
    }

    void ProcessCalltracer::GetMod0(const U8 *begin) {
        const auto *modheader{begin + *reinterpret_cast<const U32 *>(begin + 4)};
        if (*reinterpret_cast<const U32 *>(modheader) != ConstMagicValue<U32>("MOD0"))
            throw exception{"Failed to locate the MOD0 section at address {}", fmt::ptr(modheader)};

        // https://docs.oracle.com/cd/E23824_01/html/819-0690/chapter6-42444.html
        const auto *dynentries{reinterpret_cast<const Elf64_Dyn *>(modheader + *reinterpret_cast<const U32 *>(modheader + 0x4))};
        static_assert(sizeof(Elf64_Dyn) == 0x10);

        const char *stringtable{};
        const Elf64_Sym *symbols{};
        U64 entrysize{};

        for (; dynentries->d_tag; dynentries++) {
            if (dynentries->d_tag == DT_STRTAB)
                stringtable = reinterpret_cast<const char *>(begin + dynentries->d_un.d_ptr);
            else if (dynentries->d_tag == DT_SYMTAB)
                symbols = reinterpret_cast<const Elf64_Sym *>(begin + dynentries->d_un.d_ptr);
            else if (dynentries->d_tag == DT_SYMENT)
                entrysize = dynentries->d_un.d_val;
        }

        NX_ASSERT(entrysize == sizeof(Elf64_Sym));
        for (; reinterpret_cast<const char *>(symbols) < stringtable; symbols++) {
            const auto *manglename{&stringtable[symbols->st_name]};

            I32 status;
            const std::unique_ptr<char, decltype(std::free)*> demangled{__cxxabiv1::__cxa_demangle(manglename, nullptr, nullptr, &status), std::free};
            solvedsyms.push_back(std::make_pair(symbols, std::string{!status ? demangled.get() : manglename}));
        }

    }
}
