#include <boost/fiber/operations.hpp>

#include <common/container.h>
#include <device/capabilities.h>

#include <kernel/kernel.h>
#include <kernel/types/kthread.h>
#include <kernel/threads/kscheduler.h>


namespace FastNx::Kernel::Threads {
    KScheduler::KScheduler(Kernel &_kernel) : kernel(_kernel) {
        coreslist.resize(4);
    }
    void KScheduler::StartThread(const U32 idealcore) {
        auto threadit{coreslist.begin()};
        for (auto index{idealcore}; index; index--)
            ++threadit;

        if (threadit == coreslist.end())
            return;
        Device::SetCore(idealcore);
        kernel.CreateJit(idealcore);

        while (threadit->enable) {
            if (ismultithread) {
                std::unique_lock lock{threadit->mutex};
                threadit->available.wait(lock);
            }

            auto &preemplist{threadit->preempting};
            while (!preemplist.empty()) {
                const auto &thread{preemplist.begin()};
                (*thread)->ResumeThread();
                preemplist.splice(preemplist.end(), preemplist, thread);
            }

            Yield();
        }
        Quit();
    }
    void KScheduler::Emplace(const std::shared_ptr<Types::KThread> &thread) {
        std::scoped_lock lock{schedulermutex};
        preemplist.emplace_back(thread);
    }

    void KScheduler::PreemptThread(const U32 idealcore, const std::shared_ptr<Types::KThread> &thread) {
        std::shared_lock lock{schedulermutex};

        auto coresit{coreslist.begin()};
        for (U32 start{}; start < idealcore; ++start)
            ++coresit;
        if (ismultithread && !rthrlist.contains(&*coresit)) {
            rthrlist.emplace(&*coresit, std::thread(&KScheduler::StartThread, this, idealcore));
        } else if (!fiberlist.contains(&*coresit))
            fiberlist.emplace(&*coresit, boost::fibers::fiber(&KScheduler::StartThread, this, idealcore));

        {
            std::scoped_lock threadlock{coresit->mutex};
            coresit->preempting.emplace_front(thread);
            coresit->available.notify_one();
        }

        if (ismultithread)
            return;
        auto fiberit{fiberlist.begin()};
        while (fiberit->first != &*coresit)
            ++fiberit;

        if (fiberit->second.joinable())
            fiberit->second.join();
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

    void KScheduler::Yield() {
        Reeschedule();
    }

    void KScheduler::Reeschedule() {
        for (auto threadit{preemplist.begin()}; threadit != preemplist.end(); ) {
            const auto &osthread{(*threadit)};
            if (!osthread->state)
                if (threadit = preemplist.erase(threadit); threadit != preemplist.end())
                    continue;
            if (!osthread->usertls || !osthread->entrypoint) {
                preemplist.splice(preemplist.end(), preemplist, threadit);
                continue;
            }

            for (const auto &[cpuid, context]: std::ranges::views::enumerate(coreslist)) {
                const auto optionalcore{osthread->desiredcpu};
                if (optionalcore != cpuid)
                    continue;

                bool preempted{};
                for (const auto &thread: context.preempting)
                    if (thread == osthread)
                        preempted = true;

                if (preempted)
                    break;
                last = std::exchange(next, osthread);
                preemplist.splice(preemplist.end(), preemplist, threadit);

                PreemptThread(optionalcore, osthread);
            }

            ++threadit;
        }
    }
}
