#include <print>
#include <string>
#include <optional>
#include <unistd.h>

#include <common/exception.h>
#include <common/container.h>
#include <fs_sys/refs/directory_file_access.h>
#include <fs_sys/refs/buffered_file.h>
#include <common/async_logger.h>

#include <horizon/nx_apps.h>

#include <core/application.h>


namespace FastNx::Core {
    U64 GetCoreNumber() {
        return sched_getcpu();
    }

    U64 GetProcessId() {
        return getpid();
    }

    Application::Application() {
        std::optional<std::string> osname;
#if defined(__linux__)
        FsSys::ReFs::DirectoryFileAccess release{"/etc"};
        if (const auto files{release.GlobAllFiles("*-release")}; !files.empty()) {
            osname.emplace(release.OpenFile(files.back())->ReadLine());
        }
#endif

        const auto current{std::filesystem::current_path()};
        AsyncLogger::Success("FastNx application started on core {} with PID {} in directory {}", GetCoreNumber(), GetProcessId(), FsSys::GetPathStr(current));

        AsyncLogger::Success("Operating system name: {}", *osname);
    }

    Application::~Application() {
        if (assets)
            assets->Destroy();
    }

    void Application::Initialize() {
        std::vector<std::pair<I32, std::string>> _procs;
        _procs.reserve(200);
        if (const FsSys::ReFs::DirectoryFileAccess procsdir{"/proc"}) {
            for (const auto &proc: procsdir.ListAllTopLevelFiles()) {
                const auto &_pathpid{FsSys::GetPathStr(proc)};
                if (const auto &pidstr{_pathpid.substr(_pathpid.find_last_of('/') + 1)}; isdigit(pidstr.front()))
                    if (FsSys::ReFs::BufferedFile status{proc / "status"})
                        _procs.emplace_back(strtoul(GetDataArray(pidstr), nullptr, 10), status.ReadLine());
            }
        }

        for (const auto &[_pid, _program]: _procs) {
            static const auto fastpid{getpid()};
            if (_pid != fastpid && _program.contains(process))
                throw exception{"More than one instance of FastNx cannot coexist at the same time"};
        }

        assets = std::make_shared<Assets>();
        if (!assets)
            return;
        assets->Initialize();

        keys = std::make_shared<Horizon::KeySet>(*assets->keys, *assets->tiks);
        settings = std::make_shared<AppSetup>(assets);
        switchnx = std::make_shared<Horizon::SwitchNs>(keys);

        assets->LoadGamesLists();
        switchnx->GetLoaders(assets->GetAllGames());
    }

    void Application::DumpAllLogos() const {
        const auto logos{Horizon::GetAppsPublicLogo(switchnx)};
        const auto temp{std::filesystem::temp_directory_path()};
        // ReSharper disable once CppUseStructuredBinding
        for (const auto &logo: logos) {
            const auto logofilename{temp / fmt::format("{:X}.jpeg", logo.first)};
            if (FsSys::ReFs::BufferedFile writable{logofilename, 0, FsSys::FileModeType::WriteOnly, true})
                writable.WriteSome(ToSpan(logo.second));
        }
    }

    void Application::LoadFirstPickedGame() const {
        const auto gamefile = [&] -> std::optional<FsSys::FsPath> {
            const auto files{assets->GetAllGames()};
            if (files.empty())
                return std::nullopt;
            const auto application{assets->GetAllGames().front()};
            if (application.empty())
                return std::nullopt;

            if (const auto filename{application.filename()}; application.has_filename())
                AsyncLogger::Info("Loading the game from path: {}", FsSys::GetPathStr(filename));
            return application;
        }();
        if (gamefile)
            switchnx->LoadApplicationFile(*gamefile);
    }

    std::shared_ptr<Application> GetContext() {
        static std::shared_ptr<Application> context{};
        if (!context)
            context = std::make_shared<Application>();
        return context;
    }
}
