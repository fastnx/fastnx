#include <boost/fiber/operations.hpp>

#include <common/container.h>
#include <kernel/types/kthread.h>
#include <kernel/threads/kscheduler.h>


namespace FastNx::Kernel::Threads {
    void KScheduler::Emplace(const std::shared_ptr<Types::KThread> &thread) {
        if (ismultithread)
            rthrlist.emplace_back(&Types::KThread::StartThread, thread);
        else
            fiberlist.emplace_back(&Types::KThread::StartThread, thread);
    }

    void KScheduler::SetThreadName(const std::string &threadname) {
        if (ismultithread) {
            pthread_setname_np(pthread_self(), GetDataArray(threadname));
            return;
        }

        const auto fiberid{boost::this_fiber::get_id()};
        fibersname.insert_or_assign(fiberid, threadname);
    }

    void KScheduler::Quit() {
        if (ismultithread)
            for (auto &thread: rthrlist)
                if (thread.get_id() == std::this_thread::get_id())
                    thread.detach();

        for (auto &fiber: fiberlist)
            if (fiber.get_id() == boost::this_fiber::get_id())
                fiber.detach();
    }

    void KScheduler::Sleep(const std::chrono::milliseconds &value) const {
        if (ismultithread)
            std::this_thread::sleep_for(value);
        else
            boost::this_fiber::sleep_for(value);
    }
}
