#pragma once

#include <memory>
#include <boost/fiber/fiber.hpp>

#include <kernel/types.h>
namespace FastNx::Kernel::Threads {

    class KScheduler {
    public:
        explicit KScheduler(Kernel &_kernel) : kernel(_kernel) {}

        void Emplace(const std::shared_ptr<Types::KThread> &thread);

        void SetThreadName(const std::string &threadname);
        void Quit();
        void Sleep(const  std::chrono::milliseconds &value) const;

        std::weak_ptr<Types::KThread> last;
        std::weak_ptr<Types::KThread> next;

        bool ismultithread{};

        using Id = boost::fibers::fiber::id;
        std::map<Id, std::string> fibersname;

    private:
        std::list<std::thread> rthrlist;
        std::list<boost::fibers::fiber> fiberlist;

        Kernel &kernel;
    };
}