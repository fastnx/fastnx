#include <boost/fiber/operations.hpp>

#include <common/container.h>
#include <device/capabilities.h>

#include <kernel/types/kthread.h>
#include <kernel/threads/kscheduler.h>


namespace FastNx::Kernel::Threads {
    KScheduler::KScheduler(Kernel &_kernel) : kernel(_kernel) {
        coresctx.resize(4);
    }
    void KScheduler::StartThread(U32 idealcore) {
        Device::SetCore(idealcore);

        auto threadit{coresctx.begin()};
        while (idealcore--)
            ++threadit;

        while (threadit->enable) {
            if (ismultithread) {
                std::unique_lock lock{threadit->mutex};
                threadit->available.wait(lock);
            }

            if (threadit->osthread)
                threadit->osthread->ResumeThread();
            else if (!ismultithread)
                boost::this_fiber::yield();
        }
        Quit();
    }

    void KScheduler::Emplace(const std::shared_ptr<Types::KThread> &thread) {
        std::scoped_lock lock{schedulermutex};
        preemplist.emplace_back(thread);
    }

    void KScheduler::PreemptNextThread(const U32 idealcore, const std::shared_ptr<Types::KThread> &thread) {
        // Searching for one or more threads that were just created
        std::shared_lock lock{schedulermutex};

        auto coresit{coresctx.begin()};
        for (U32 start{}; start < idealcore; ++start)
            ++coresit;
        if (ismultithread && !rthrlist.contains(&*coresit)) {
            rthrlist.emplace(&*coresit, std::thread(&KScheduler::StartThread, this, idealcore));
        } else if (!fiberlist.contains(&*coresit))
            fiberlist.emplace(&*coresit, boost::fibers::fiber(&KScheduler::StartThread, this, idealcore));

        {
            std::scoped_lock threadlock{coresit->mutex};
            if (coresit->osthread != thread)
                coresit->osthread = thread;
            coresit->available.notify_one();
        }

        if (!ismultithread)
            if (fiberlist.contains(&*coresit))
                fiberlist[&*coresit].join();
    }

    void KScheduler::KillThread(const std::shared_ptr<Types::KThread> &thread) {
        auto threadit{preemplist.begin()};
        for (; threadit != preemplist.end(); ++threadit) {
            if (*threadit != thread)
                continue;

            thread->state = {};
            preemplist.erase(threadit);
            break;
        }
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
        std::shared_lock lock{schedulermutex};
        if (ismultithread)
            for (auto &thread: rthrlist | std::views::values)
                if (thread.get_id() == std::this_thread::get_id())
                    thread.detach();

        for (auto &fiber: fiberlist | std::views::values)
            if (fiber.get_id() == boost::this_fiber::get_id())
                fiber.detach();
    }

    void KScheduler::Sleep(const std::chrono::milliseconds &value) const {
        if (ismultithread)
            std::this_thread::sleep_for(value);
        else
            boost::this_fiber::sleep_for(value);
    }

    void KScheduler::Reeschedule() {
        for (auto threadit{preemplist.begin()}; threadit != preemplist.end(); ) {
            const auto &osthread{(*threadit)};
            if (!osthread->state)
                if (threadit = preemplist.erase(threadit); threadit != preemplist.end())
                    continue;
            if (!osthread->usertls || !osthread->entrypoint)
                preemplist.splice(preemplist.end(), preemplist, threadit);

            last = std::exchange(next, osthread);
            preemplist.splice(preemplist.end(), preemplist, threadit);

            const auto optionalcore{osthread->desiredcpu};
            PreemptNextThread(optionalcore, osthread);

            ++threadit;
        }
    }
}
