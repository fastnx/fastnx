#include <cassert>
#include <pwd.h>
#include <unistd.h>

#include <fs_sys/assets.h>

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

    {
        const auto procAssets{std::make_shared<FastNx::FsSys::Assets>()};
    }
}
