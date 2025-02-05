#include <cassert>
#include <pwd.h>
#include <unistd.h>

#include <print>

#include <core/application.h>
#include <core/assets.h>
#include <device/capabilities.h>
FastNx::FsSys::FsPath GetUserDir() {
    if (const auto *const user{getpwuid(getuid())})
        return user->pw_dir;
    return {};
}
int main() {
    assert(FastNx::FsSys::IsInsideOf(std::filesystem::current_path(), GetUserDir()));
    [[maybe_unused]] const auto isPrivileged = [] {
        const auto uid{getuid()};
        return !uid || geteuid() != uid;
    }();
    assert(!isPrivileged);
    assert(pthread_self() && getpid());

    if (const auto [aspects, rank] = FastNx::Device::IsArchSuitable(); !aspects.empty())
        std::println("Features supported by the Host system: {}, Your rank {}", aspects, rank);

    {
        [[maybe_unused]] const auto application{std::make_shared<FastNx::Core::Application>()};
        application->Initialize();
    }
}
