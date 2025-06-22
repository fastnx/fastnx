#include <atomic>
#include <pwd.h>
#include <unistd.h>

#include <iostream>
#include <print>
#include <boost/program_options.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#include <core/application.h>
#include <core/cache.h>
#include <device/capabilities.h>

#include <fs_sys/refs/buffered_file.h>
#include <common/async_logger.h>


namespace FastNx {
    FsSys::FsPath GetUserDir() {
        if (const auto *user{getpwuid(getuid())})
            return user->pw_dir;
        return {};
    }
    bool IsProcessPrivileged() {
        const auto uid{getuid()};
        return !uid || geteuid() != uid;
    }
}

namespace bpo = boost::program_options;
bpo::options_description fastopts("Allowed options");

bool lockmmap;

using namespace FastNx;

I32 main(const I32 argc, const char **argv) {
    fastopts.add_options()
        ("help", "Display this table")
        ("lock-mmap", "Block all future memory mappings in RAM")
        ("clear-cache", "Removes all cache files created by the emulator")
        ("export", "Exports all content produced by the system");

    bpo::variables_map vm;
    store(parse_command_line(argc, argv, fastopts), vm);
    notify(vm);

    if (vm.contains("help")) {
        fastopts.print(std::cerr);
        return EXIT_SUCCESS;
    }
    lockmmap = vm.contains("lock-mmap");
    if (lockmmap)
        Device::LockAll();

    NX_ASSERT(FastNx::FsSys::ReFs::BufferedFile("app_lock", 0, FastNx::FsSys::FileModeType::WriteOnly, true));

    boost::interprocess::file_lock flock("app_lock");

    {
        const std::atomic<bool> test;
        NX_ASSERT(test.is_lock_free());
    }

    if (!FsSys::IsInsideOf(std::filesystem::current_path(), GetUserDir()))
        return EXIT_FAILURE;

    if (IsProcessPrivileged())
        return EXIT_FAILURE;
    if (pthread_self() == 0 && getpid() == 0)
        return EXIT_FAILURE;

    BuildAsyncLogger();
    if (const auto &[aspects, ranking] = Device::GetArchAspects(); !aspects.empty()) {
        AsyncLogger::Success("Features supported by the host system: {}, your rank {}", aspects, ranking);
    }

    if (const auto &application{Core::GetContext()}) {
        application->Initialize();
        if (vm.contains("clear-cache")) {
            Core::ClearCache(application);
        } else if (vm.contains("export")) {
            Core::ExportCache(application);
        } else {
            application->LoadFirstPickedGame();
            // application->DumpAllLogos();
        }
    }

    std::atexit([] {
        logger->FlushBuffers();
    });
}
