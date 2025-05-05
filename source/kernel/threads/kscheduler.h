#pragma once

#include <kernel/types.h>
namespace FastNx::Kernel::Threads {

    class KScheduler {
    public:
        explicit KScheduler(Kernel &_kernel) : kernel(_kernel) {}

        Kernel &kernel;
    };
}