#include <print>
#include <string>
#include <optional>
#include <unistd.h>

#include <common/container.h>
#include <fs_sys/refs/editable_directory.h>
#include <core/application.h>

FastNx::U64 GetCoreNumber() {
    return sched_getcpu();
}
FastNx::U64 GetProcessId() {
    return getpid();
}

FastNx::Core::Application::Application() {

    std::optional<std::string> systemsname;
#if defined(__linux__)
    FsSys::ReFs::EditableDirectory release{"/etc"};
    if (const auto files{release.BlobAllFiles("*-release")}; !files.empty()) {
        systemsname.emplace(files.front().string());
    }
#endif

    const auto current{std::filesystem::current_path()};
    std::println("FastNx application started on core {} with PID {} in directory {}", GetCoreNumber(), GetProcessId(), LandingOf(current));
    if (const auto sysstr{systemsname.value()}; systemsname)
        std::println("Operating system name: {}", sysstr);
}
