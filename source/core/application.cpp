#include <print>
#include <string>
#include <optional>
#include <unistd.h>

#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <fs_sys/refs/huge_file.h>
#include <core/application.h>

namespace FastNx::Core {
    U64 GetCoreNumber() {
        return sched_getcpu();
    }

    U64 GetProcessId() {
        return getpid();
    }

    Application::Application() : _switch(std::make_shared<Horizon::SwitchNs>()) {
        std::optional<std::string> systemsname;
#if defined(__linux__)
        FsSys::ReFs::EditableDirectory release{"/etc"};
        if (const auto files{release.GlobAllFiles("*-release")}; !files.empty()) {
            systemsname.emplace(release.OpenFile(files.front())->ReadLine());
        }
#endif

        const auto current{std::filesystem::current_path()};
        std::println("FastNx application started on core {} with PID {} in directory {}", GetCoreNumber(), GetProcessId(), LandingOf(current));

        if (const auto sysstr{systemsname.value()}; systemsname)
            std::println("Operating system name: {}", sysstr);
    }

    Application::~Application() {
        if (filesAm)
            filesAm->Destroy();
    }

    void Application::Initialize() {
        if (auto fsAssets{std::make_shared<Assets>()})
            filesAm.swap(fsAssets);
        filesAm->Initialize();
    }

    void Application::LoadFirstPickedGame() const {
        const auto gamefile = [&]-> FsSys::VfsBackingFilePtr {
            const auto &runnable{filesAm->GetAllGames()};
            if (runnable.empty())
                return nullptr;

            const auto &_gamePath{runnable.front()};
            if (const auto _filename{_gamePath.filename()}; _gamePath.has_filename())
                std::println("Loading the game from path: {}", _filename.string());
            return std::make_shared<FsSys::ReFs::HugeFile>(_gamePath);
        }();
        if (gamefile != nullptr)
            _switch->LoadApplicationFile(gamefile);
    }
}
