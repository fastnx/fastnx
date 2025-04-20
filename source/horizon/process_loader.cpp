#include <boost/align/align_up.hpp>
#include <common/container.h>
#include <common/async_logger.h>
#include <common/format.h>
#include <horizon/switch_ns.h>
#include <horizon/process_loader.h>

#include <kernel/svc/types.h>
#include <fs_sys/npdm.h>



namespace FastNx::Horizon {
    void ProcessLoader::GetCodeSet(Kernel::ProcessCodeLayout &codeset, const std::shared_ptr<Loaders::NsoFmt> &nso, const bool copyimage) {
        auto baseoffset{codeset.offset};

        AsyncLogger::Puts("Loading NSO: {:^8}", GetPathStr(nso->backing));
        for (const auto &[type, section]: nso->secsalign) {
            const auto [offset, size] = section;

            if (copyimage) {
                if (std::span codeout{codeset.binaryimage.data() + baseoffset, size}; codeout.size())
                    Copy(codeout, nso->secsmap[type]);
            }
            const bool isdata{type == Loaders::NsoSectionType::Data};
            AsyncLogger::Puts("{}: {:016X}, ", isdata ? ".data" : type == Loaders::NsoSectionType::Text ? ".text" : ".ro", baseoffset);
            if (isdata) // The size of the data section is aligned together with the size of the .bss section
                baseoffset += boost::alignment::align_up(size + nso->bsssize, Kernel::SwitchPageSize);
            else
                baseoffset += boost::alignment::align_up(size, Kernel::SwitchPageSize);
        }
        if (!copyimage)
            AsyncLogger::ClearLine();
        else
            AsyncLogger::Puts("\n");

        codeset.offset = baseoffset;
    }

    void ProcessLoader::Load() {
        if (const auto kernel{switchnx->kernel})
            process = kernel->CreateProcess();

        auto &tables{process->kernel.tables};
        if (!tables)
            tables.emplace(process->kernel);

        const auto loader{switchnx->loader};
        std::optional<FsSys::Npdm> npdm;
        if (const auto npdmfile{loader->appdir->GetNpdm()})
            npdm.emplace(npdmfile);

        Kernel::ProcessCodeLayout codeset{};

        if (loader->type == Loaders::AppType::NspEs) {
            Kernel::ProcessCodeLayout maskset{}; // Mask used to discover the total size and alignment of the binaries

            const auto exefs{loader->appdir->GetExefs()};
            const auto modules{Loaders::NsoFmt::LoadModules(exefs)};

            if (const auto rtld{modules.front()})
                codeset.start = rtld->secsalign.front().second.first;
            for (const auto &nsomodule: modules)
                GetCodeSet(maskset, nsomodule);

            codeset.binaryimage.resize(maskset.offset);
            for (const auto &nsomodule: modules)
                GetCodeSet(codeset, nsomodule, true);
        }

        AsyncLogger::Info("Aligned binary layout size: {}", FormatSize{codeset.binaryimage});
        Kernel::Svc::CreateProcessParameter process{};
        std::memcpy(&process.flagsint, &npdm->procflags, 1);

        process.codestart = codeset.start;
        if (codeset.offset)
            process.codenumpages = (codeset.offset - codeset.start) / Kernel::SwitchPageSize;
        process.titleid = loader->GetTitleId();

        // tables->InitilizeMemoryForProcess(process, codeset);
    }
}
