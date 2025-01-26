#include <print>
#include <string>
#include <optional>
#include <unistd.h>

#include <base/container.h>
#include <fs_sys/ssd/editable_directory.h>
#include <core/application.h>

uint64_t GetCoreNumber() {
    return sched_getcpu();
}
uint64_t GetProcessId() {
    return getpid();
}

FastNx::Core::Application::Application() {

    const std::optional<std::string> systemsname;
#if defined(__linux__)
    FsSys::SSD::EditableDirectory release{"/etc"};
    if (const auto files{release.BlobAllFiles("*-release")}; !files.empty()) {
    }
#endif

    const auto current{std::filesystem::current_path()};
    std::println("FastNx application started on core {} with PID {} in directory {}", GetCoreNumber(), GetProcessId(), LandingOf(current));
    if (const auto sysstr = systemsname.value(); !sysstr.empty())
        std::println("Operating system name: {}", sysstr);
}
