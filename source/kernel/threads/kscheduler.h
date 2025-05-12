#pragma once

#include <memory>
#include <shared_mutex>
#include <boost/fiber/fiber.hpp>

#include <kernel/types.h>
namespace FastNx::Kernel::Threads {
    struct CoreContext {
        std::mutex mutex;
        std::condition_variable available;
        bool enable{true};

        std::list<std::shared_ptr<Types::KThread>> preempting;
    };

    class KScheduler {
    public:
        explicit KScheduler(Kernel &_kernel);

        void StartThread(U32 idealcore);

        void Emplace(const std::shared_ptr<Types::KThread> &thread);
        void PreemptThread(U32 idealcore, const std::shared_ptr<Types::KThread> &thread);
        void KillThread(const std::shared_ptr<Types::KThread> &thread);

        void SetThreadName(const std::string &threadname);
        void Sleep(const std::chrono::milliseconds &value) const;
        void Yield();

        void Reeschedule();
        void Quit();

        std::list<CoreContext> coresctx;

        std::shared_ptr<Types::KThread> last;
        std::shared_ptr<Types::KThread> next;

        bool ismultithread{};

        std::map<boost::fibers::fiber::id, std::string> fibersname;

    private:
        std::list<std::shared_ptr<Types::KThread>> preemplist;
        std::unordered_map<CoreContext*, std::thread> rthrlist;
        std::unordered_map<CoreContext*, boost::fibers::fiber> fiberlist;
        std::shared_mutex schedulermutex;

        Kernel &kernel;
    };
}