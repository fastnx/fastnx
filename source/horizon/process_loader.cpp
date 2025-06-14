#include <boost/align/align_up.hpp>
#include <common/container.h>
#include <common/async_logger.h>
#include <common/format.h>
#include <horizon/switch_ns.h>
#include <horizon/process_loader.h>

#include <kernel/svc/types.h>

#include <debug/readonly_buffer.h>


namespace FastNx::Horizon {
    std::map<U64, FsSys::FsPath> nsolist;
    FsSys::FsPath GetNsoName(const U64 basenso) {
        if (nsolist.contains(basenso))
            return nsolist[basenso];
        return {};
    }

    void ProcessLoader::GetCodeSet(Kernel::ProcessCodeLayout &codeset, const std::shared_ptr<Loaders::NsoFmt> &nso) {
        auto baseoffset{codeset.offset};

        AsyncLogger::Puts("Loading NSO: {:^8}", GetPathStr(nso->backing));
        nsolist.emplace(baseoffset, GetPathStr(nso->backing));
        for (const auto &[type, section]: nso->secsalign) {
            const auto [offset, size] = section;

            codeset.procimage.emplace_back(baseoffset, std::move(nso->secsmap[type]));

            // The size of the data section is aligned together with the size of the .bss section
            const bool isdata{type == Loaders::NsoSectionType::Data};
            if (isdata)
                codeset.bsslayoutsize.emplace_back(nso->bsssize);
            AsyncLogger::Puts("{}: {:016X}, ", isdata ? ".data" : type == Loaders::NsoSectionType::Text ? ".text" : ".ro", baseoffset);

            baseoffset += boost::alignment::align_up(isdata ? size + nso->bsssize : size, Kernel::SwitchPageSize);
        }
        AsyncLogger::Puts("\n");
        codeset.offset = baseoffset;
    }

    void ProcessLoader::SetProcessMemory(Kernel::ProcessCodeLayout &&codeset) const {
        const U64 startoffset{codeset.start};
        U64 bintype{};
        NX_ASSERT(codeset.procimage.size() % 3 == 0);

        U64 basetextoffset{};
        for (const auto &[offset, content]: codeset.procimage) {
            auto size{boost::alignment::align_up(content.size(), Kernel::SwitchPageSize)};

            const auto permission = [&] {
                if (!bintype)
                    return Kernel::Permission::Text;
                if (bintype == 1)
                    return Kernel::Permission::Ro;
                return Kernel::Permission::Data;
            }();
            if (!bintype)
                basetextoffset = offset;
            bintype++;
            U64 bsssize{};
            if (permission == Kernel::Permission::Data) {
                bsssize = codeset.bsslayoutsize.front();
                size = boost::alignment::align_up(content.size() + bsssize, Kernel::SwitchPageSize);
            }
            process->memory->MapCodeMemory(startoffset + offset, size, content);
#if !NDEBUG
            if (permission == Kernel::Permission::Ro) {
                const auto &strings{Debug::Strings(process->memory->code.begin().base() + startoffset + offset, size)};
                const std::string_view _contentstr{strings.data(), strings.size()};
                if (_contentstr.empty())
                    return;

                AsyncLogger::Info("Readable content mapped in memory: {}", std::string_view(strings.data(), strings.size()));
                Debug::PrintTopics(_contentstr, GetNsoName(basetextoffset));
            }
#endif

            process->memory->SetMemoryPermission(startoffset + offset, size, permission);

            if (!bsssize)
                continue;
            process->memory->FillMemory(startoffset + offset + content.size(), 0, bsssize);
            codeset.bsslayoutsize.pop_front();
            bintype = 0;
        }
    }

    bool ValidateSet(const Kernel::ProcessCodeLayout &codeset) {
        return codeset.offset && !codeset.procimage.empty() && !codeset.bsslayoutsize.empty();
    }

    void ProcessLoader::Load() {
        if (const auto &kernel{switchnx->kernel})
            process = kernel->CreateProcess();


        const auto loader{switchnx->loader};
        if (const auto npdmfile{loader->appdir->GetNpdm()})
            npdm.emplace(npdmfile);

        Kernel::ProcessCodeLayout codeset{};

        if (loader->type == Loaders::AppType::NspEs) {

            const auto exefs{loader->appdir->GetExefs()};
            const auto modules{Loaders::NsoFmt::LoadModules(exefs)};

            if (const auto rtld{modules.front()})
                codeset.start = rtld->secsalign.front().second.first;
            for (const auto &nsomodule: modules)
                GetCodeSet(codeset, nsomodule);
        }

        const auto processize = [&] -> U64 {
            if (!codeset.procimage.empty())
                return codeset.offset;
            return {};
        }();
        AsyncLogger::Info("Process memory layout size: {}", FormatSize{processize});
        Kernel::Svc::CreateProcessParameter parameters{};
        std::memcpy(&parameters.flagsint, &npdm->procflags, 1);

        parameters.codestart = codeset.start;
        if (codeset.offset)
            parameters.codenumpages = (codeset.offset - codeset.start) / Kernel::SwitchPageSize;
        parameters.titleid = loader->GetTitleId();

        parameters.enbaslr = true;

        const auto &memory{switchnx->kernel->memory};
        memory->InitializeForProcess(process, parameters);

        if (ValidateSet(codeset))
            SetProcessMemory(std::move(codeset));
    }
}
