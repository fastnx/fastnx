
#include <iostream>
#include <shared_mutex>
#include <fmt/chrono.h>
#include <boost/algorithm/string.hpp>

#include <fs_sys/standard_file.h>
#include <common/async_logger.h>
namespace FastNx {
    std::shared_ptr<AsyncLogger> BuildAsyncLogger(std::optional<FsSys::ReFs::EditableDirectory> logdir) {
        std::optional<fmt::memory_buffer> logs;
        U64 count{};
        if (logger) {
            logs.emplace(std::move(logger->fmtlists));
            count = logger->count;
        }
        if (!logdir) {
            logger = std::make_shared<AsyncLogger>(std::cout); // Only streams valid for the lifetime of the process are valid here
            logger->threshold = 4;
        } else {
            const auto filename{logdir->path / std::format("fastnx-{:%m-%d-%y}.log", std::chrono::system_clock::now())};
            if (const auto logfile{logdir->OpenFile(filename, FsSys::FileModeType::WriteOnly)})
                logger = std::make_shared<AsyncLogger>(logfile);
        }
        if (logs)
            logger->fmtlists.append(std::move(*logs));
        logger->count = count;
        return logger;
    }

    AsyncLogger::AsyncLogger(std::ostream &output) :
        outback(std::make_shared<FsSys::StandardFile>(output)) {}

    AsyncLogger::AsyncLogger(const FsSys::VfsBackingFilePtr &file) : outback(file) {}

    void AsyncLogger::FlushBuffers() {
        std::shared_lock guard(lock);
        for (U64 _offset{}; _offset < fmtlists.size() && count; count--) {
            const auto *begin{fmtlists.begin() + _offset};
            const std::string_view line(begin, strchr(begin, '\n') + 1);
            outback->WriteType(line.data(), line.size(), _offset);
            _offset += line.size();
        }
        NX_ASSERT(count == 0);
        fmtlists.clear();
    }

    void AsyncLogger::Log(std::string &&fmtmsg) {
        fmt::format_to(std::back_inserter(fmtlists), "{}", std::move(fmtmsg));
        if (fmtlists.size() && *(fmtlists.end() - 1)  == '\n')
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
