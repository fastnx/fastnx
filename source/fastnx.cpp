#include <pwd.h>
#include <unistd.h>

#include <iostream>
#include <print>
#include <boost/program_options.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#include <core/application.h>
#include <device/capabilities.h>

#include <fs_sys/refs/buffered_file.h>
#include <common/async_logger.h>


FastNx::FsSys::FsPath GetUserDir() {
    if (const auto *user{getpwuid(getuid())})
        return user->pw_dir;
    return {};
}
bool IsProcessPrivileged() {
    const auto uid{getuid()};
    return !uid || geteuid() != uid;
}

namespace boost_po = boost::program_options;
using namespace FastNx;
boost_po::options_description fastopts("Allowed options");

bool disableswap;

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
    disableswap = vm.contains("disable-disk-swap");
    if (disableswap)
        Device::LockAllMapping();

    NX_ASSERT(FsSys::ReFs::BufferedFile("app_lock", 0, FsSys::FileModeType::WriteOnly, true));

    boost::interprocess::file_lock flock("app_lock");
    if (!FsSys::IsInsideOf(std::filesystem::current_path(), GetUserDir()))
        return -1;

    if (IsProcessPrivileged())
        return -1;
    if (pthread_self() == 0 && getpid() == 0)
        return -1;

    BuildAsyncLogger();
    if (const auto &[aspects, ranking] = Device::GetArchAspects(); !aspects.empty()) {
        AsyncLogger::Success("Features supported by the Host system: {}, Your rank {}", aspects, ranking);
    }

    if (const auto application{std::make_shared<Core::Application>()}) {
        application->Initialize();
        application->LoadFirstPickedGame();
    }

    logger->FlushBuffers();
}
