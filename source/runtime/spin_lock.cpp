#include <thread>
#include <runtime/spin_lock.h>

namespace FastNx::Runtime {
    void SpinLock::lock() {
        U64 count{};
        while (!try_lock()) {
            if (count++ > 1000)
                std::this_thread::yield();
        }

    }
    bool SpinLock::try_lock() { U32 expected{};
        return lockval.compare_exchange_strong(expected, 1, std::memory_order_acq_rel);
    }
    void SpinLock::unlock() {
        lockval.fetch_and(0, std::memory_order_release);
    }
}
