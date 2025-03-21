#include <print>
#include <string>
#include <optional>
#include <unistd.h>

#include <common/exception.h>
#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <fs_sys/refs/huge_file.h>
#include <fs_sys/refs/buffered_file.h>
#include <common/async_logger.h>
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
        FsSys::ReFs::EditableDirectory release{"/etc"};
        if (const auto files{release.GlobAllFiles("*-release")}; !files.empty()) {
            osname.emplace(release.OpenFile(files.front())->ReadLine());
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
        if (const FsSys::ReFs::EditableDirectory procsdir{"/proc"}) {
            for (const auto &proc: procsdir.ListAllTopLevelFiles()) {
                const auto &_pathpid{FsSys::GetPathStr(proc)};
                if (const auto &pidstr{_pathpid.substr(_pathpid.find_last_of('/') + 1)}; isdigit(pidstr.front()))
                    if (FsSys::ReFs::BufferedFile status{proc / "status"})
                        _procs.emplace_back(strtoul(GetDataArray(pidstr), nullptr, 10), status.ReadLine());
            }
        }

        for (const auto &[_pid, _program]: _procs) {
            static const auto fastpid{getpid()};
            if (_pid != fastpid && _program.contains("fastnx"))
                throw exception{"More than one instance of Fastnx cannot coexist at the same time"};
        }

        assets = std::make_shared<Assets>();
        if (!assets)
            return;
        assets->Initialize();

        keys = std::make_shared<Horizon::KeySet>(*assets->keys, *assets->tiks);
        _switch = std::make_shared<Horizon::SwitchNs>(keys);
    }

    void Application::LoadFirstPickedGame() const {
        const auto gamefile = [&]-> FsSys::VfsBackingFilePtr {
            const auto &runnable{assets->GetAllGames()};
            if (runnable.empty())
                return nullptr;

            const auto &_gamePath{runnable.front()};
            if (const auto _filename{_gamePath.filename()}; _gamePath.has_filename())
                AsyncLogger::Info("Loading the game from path: {}", FsSys::GetPathStr(_filename));
            return std::make_shared<FsSys::ReFs::HugeFile>(_gamePath);
        }();
        if (gamefile != nullptr)
            _switch->LoadApplicationFile(gamefile);
    }
}
