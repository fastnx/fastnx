#pragma once

#include <atomic>

#include <common/types.h>
namespace FastNx::Runtime {
    // https://github.com/facebook/folly/blob/main/folly/synchronization/RWSpinLock.h
    class SpinLock {
    public:
        SpinLock() = default;
        void lock();
        bool try_lock();
        void unlock();

        std::atomic_flag lockval{ATOMIC_FLAG_INIT};
    };

}