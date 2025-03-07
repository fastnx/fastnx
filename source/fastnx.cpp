#include <pwd.h>
#include <unistd.h>

#include <iostream>
#include <print>
#include <boost/program_options.hpp>

#include <core/application.h>
#include <device/capabilities.h>

FastNx::FsSys::FsPath GetUserDir() {
    if (const auto *const user{getpwuid(getuid())})
        return user->pw_dir;
    return {};
}

namespace boost_po = boost::program_options;
boost_po::options_description fastopts("Allowed options");

using namespace FastNx;

bool disableSwap;

int main(const I32 argc, const char **argv) {
    fastopts.add_options()
        ("help", "Display this table")
        ("disable-disk-swap", "Block all future memory mappings in RAM");

    boost_po::variables_map vm;
    store(parse_command_line(argc, argv, fastopts), vm);
    notify(vm);

    if (vm.contains("help")) {
        fastopts.print(std::cerr);
        return 0;
    }
    disableSwap = vm.contains("disable-disk-swap");

    if (disableSwap)
        Device::LockAllMapping();

    if (!FsSys::IsInsideOf(std::filesystem::current_path(), GetUserDir()))
        return -1;

    const auto isPrivileged = [] {
        const auto uid{getuid()};
        return !uid || geteuid() != uid;
    }();
    if (isPrivileged)
        return -1;
    if (pthread_self() == 0 && getpid() == 0)
        return -1;

    if (const auto &[aspects, ranking] = Device::GetArchAspects(); !aspects.empty()) {
        std::println("Features supported by the Host system: {}, Your rank {}", aspects, ranking);
    }

    if (const auto application{std::make_shared<Core::Application>()}) {
        application->Initialize();
        application->LoadFirstPickedGame();
    }

    return 0;
}
