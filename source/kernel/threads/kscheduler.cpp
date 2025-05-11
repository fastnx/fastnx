#include <boost/fiber/operations.hpp>

#include <common/container.h>
#include <kernel/types/kthread.h>
#include <kernel/threads/kscheduler.h>

namespace FastNx::Kernel::Threads {
    [[noreturn]] void KScheduler::Entry(U32 corenumber) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_ISSET(corenumber, &cpuset);
        pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset);

        auto contextiit{contexts.begin()};
        while (corenumber--)
            ++contextiit;
        if (contextiit->osthread != next)
            contextiit->osthread = next;

        for (;;) {
            contextiit->osthread->ResumeThread();
        }
        Quit();
    }

    void KScheduler::Emplace(const std::shared_ptr<Types::KThread> &thread) {
        std::scoped_lock lock{prmutex};
        preemplist.emplace_back(thread);
    }

    void KScheduler::PreemptNextThread(const U32 idealcore, const std::shared_ptr<Types::KThread> &thread) {
        // Searching for one or more threads that were just created
        std::shared_lock lock{prmutex};

        if (ismultithread)
            rthrlist.emplace(idealcore, std::thread(&KScheduler::Entry, this, idealcore));
        else if (fiberlist.contains(idealcore))
            fiberlist.emplace(idealcore, boost::fibers::fiber(&KScheduler::Entry, this, idealcore));
    }

    void KScheduler::KillThread(const std::shared_ptr<Types::KThread> &thread) {
        auto threadit{preemplist.begin()};
        for (; threadit != preemplist.end(); ++threadit) {
            if (*threadit != thread)
                continue;

            thread->state--;
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
        std::shared_lock lock{prmutex};
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

    bool CanHosThreadRun(const std::shared_ptr<Types::KThread> &hos) {
        if (hos->state == U32{})
            return {};
        if (!hos->usertls || !hos->entrypoint)
            return {};
        return true;
    }

    void KScheduler::Reeschedule() {
        for (auto threadit{preemplist.begin()}; threadit != preemplist.end(); ) {
            if ((*threadit)->state == 0)
                if (threadit = preemplist.erase(threadit); threadit != preemplist.end())
                    continue;
            if (!CanHosThreadRun(*threadit)) {
                preemplist.splice(preemplist.end(), preemplist, threadit);
                continue;
            }

            last = std::exchange(next, *threadit);
            preemplist.splice(preemplist.end(), preemplist, threadit);

            const auto optionalcore{(*threadit)->desiredcpu};
            PreemptNextThread(optionalcore, *threadit);

            ++threadit;
        }
    }
}
