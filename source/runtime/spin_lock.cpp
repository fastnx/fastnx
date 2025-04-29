#include <thread>
#include <runtime/spin_lock.h>

namespace FastNx::Runtime {

    // The most expensive part performs a context switch
    __attribute__((always_inline)) inline void Busy() {
        sync();
        std::this_thread::yield();
    }
    __attribute__((always_inline)) inline void Pause() {
        __asm__ volatile("pause");
    }
    __attribute__((always_inline)) inline void Loop(const U64 count) {
        volatile U64 result{};
        for (U64 index{}; index < count; index++)
            result += index;
    }

    using namespace std::literals::chrono_literals;
    constexpr auto SleepFactor{50ms};
    void SpinLock::lock() {
        U64 count{};
        for (; !try_lock(); count++) {
            if (count % 10 == 0)
                Pause();

            if (count < 512)
                Loop(count);
            else if (count > 2000)
                std::this_thread::sleep_for(SleepFactor);
            else if (count > 1000)
                Busy();
        }

    }
    bool SpinLock::try_lock() {
        return lockval.test_and_set(std::memory_order_acquire) == bool{};
    }
    void SpinLock::unlock() {
        lockval.clear(std::memory_order_release);
    }
}
