#pragma once

#include <memory>
#include <shared_mutex>
#include <boost/fiber/fiber.hpp>

#include <kernel/types.h>
namespace FastNx::Kernel::Threads {
    struct ThreadContext {
        std::shared_ptr<Types::KThread> osthread;
    };

    class KScheduler {
    public:
        explicit KScheduler(Kernel &_kernel) : kernel(_kernel) {
            contexts.resize(4);
        }

        [[noreturn]] void Entry(U32 corenumber);

        void Emplace(const std::shared_ptr<Types::KThread> &thread);
        void PreemptNextThread(U32 idealcore, const std::shared_ptr<Types::KThread> &thread);
        void KillThread(const std::shared_ptr<Types::KThread> &thread);

        void SetThreadName(const std::string &threadname);
        void Sleep(const std::chrono::milliseconds &value) const;
        void Reeschedule();
        void Quit();

        std::list<ThreadContext> contexts;

        std::shared_ptr<Types::KThread> last;
        std::shared_ptr<Types::KThread> next;

        bool ismultithread{};

        std::map<boost::fibers::fiber::id, std::string> fibersname;

    private:
        std::list<std::shared_ptr<Types::KThread>> preemplist;
        std::map<U32, std::thread> rthrlist;
        std::map<U32, boost::fibers::fiber> fiberlist;
        std::shared_mutex prmutex;

        Kernel &kernel;
    };
}