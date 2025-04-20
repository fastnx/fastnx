
#include <iostream>
#include <shared_mutex>

#include <sys/ptrace.h>
#include <fmt/chrono.h>
#include <boost/algorithm/string.hpp>

#include <fs_sys/vfs/standard_file.h>
#include <fs_sys/refs/buffered_file.h>
#include <common/async_logger.h>


namespace FastNx {
    bool IsDebuggerPresent() {
        if (const auto tracer{FsSys::ReFs::GetLine("/proc/self/status", "TracerPid:")}; !tracer.empty())
            if (const auto debugger{tracer.substr(strlen("TracerPid:") + 1)}; !debugger.empty())
                if (strtoull(debugger.data(), nullptr, 10))
                    return true;

        return ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0;
    }

    std::shared_ptr<AsyncLogger> BuildAsyncLogger(FsSys::ReFs::DirectoryFileAccess *logdir) {
        logger = [&] -> std::shared_ptr<AsyncLogger> {
            if (!logdir || IsDebuggerPresent())
                return std::make_shared<AsyncLogger>(std::cout, 4); // Only streams valid for the lifetime of the process are valid here

            const auto filename{logdir->path / std::format("fastnx-{:%m-%d-%y}.log", std::chrono::system_clock::now())};
            if (const auto logfile{logdir->OpenFile(filename, FsSys::FileModeType::WriteOnly)})
                return std::make_shared<AsyncLogger>(logfile);

            return nullptr;
        }();
        return logger;
    }

    AsyncLogger::AsyncLogger(std::ostream &output, const U64 size) :
        threshold(size), backing(std::make_shared<FsSys::Vfs::StandardFile>(output)) {}

    AsyncLogger::AsyncLogger(const FsSys::VfsBackingFilePtr &file, const U64 size) : threshold(size), backing(file) {}

    void AsyncLogger::FlushBuffers() {
        std::shared_lock guard(lock);
        for (U64 offset{}; offset < fmtlists.size() && count; count--) {
            const auto *begin{fmtlists.begin() + offset};
            const std::string_view line(begin, strchr(begin, '\n') + 1);
            backing->WriteType(line.data(), line.size(), offset);
            offset += line.size();
        }
        NX_ASSERT(count == 0);
        fmtlists.clear();
    }

    void AsyncLogger::Log(std::string &&fmtmsg) {
        fmt::format_to(std::back_inserter(fmtlists), "{}", std::move(fmtmsg));
        if (fmtlists.size() && *(fmtlists.end() - 1) == '\n')
            count++;
    }

    void AsyncLogger::Log(const LogType _type, const std::source_location &source, std::string &&fmtmsg) {
        const std::string logmode = [&] {
            if (_type == LogType::Success)
                return "Success";
            if (_type == LogType::Error)
                return "Error";
            if (_type == LogType::Info)
                return "Info";
            std::unreachable();
        }();
        const auto time{std::chrono::system_clock::now()};

        std::shared_lock guard(lock);
        static std::vector<std::string_view> lists;
        lists.clear();

        const auto location = [&] {
            std::string_view filename{source.file_name()};
            split(lists, filename, boost::is_any_of("/"));
            static std::string result;
            result.clear();

            for (auto last{lists.rbegin()}; last != lists.rend() && *last != "source"; ++last) {
                if (!result.empty())
                    result.insert(0, "/");
                result.insert(0, *last);
            }
            std::format_to(std::back_inserter(result), ": {}", source.line());
            return result;
        }();
        fmt::format_to(std::back_inserter(fmtlists), "{:%X}: {} {} {}\n", time, location, logmode, std::move(fmtmsg));
        if (threshold == ++count)
            FlushBuffers();

    }
}
