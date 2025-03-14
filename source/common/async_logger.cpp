
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
        if (_logger) {
            logs.emplace(std::move(_logger->fmtlists));
            count = _logger->count;
        }
        if (!logdir) {
            _logger = std::make_shared<AsyncLogger>(std::cout); // Only streams valid for the lifetime of the process are valid here
            _logger->threshold = 4;
        } else {
            const auto filename{logdir->path / std::format("fastnx-{:%m-%d-%y}.log", std::chrono::system_clock::now())};
            if (const auto logfile{logdir->OpenFile(filename, FsSys::FileModeType::WriteOnly)})
                _logger = std::make_shared<AsyncLogger>(logfile);
        }
        if (logs)
            _logger->fmtlists.append(std::move(*logs));
        _logger->count = count;
        return _logger;
    }

    AsyncLogger::AsyncLogger(std::ostream &output) :
        outback(std::make_shared<FsSys::StandardFile>(output)) {}

    AsyncLogger::AsyncLogger(const FsSys::VfsBackingFilePtr &file) : outback(file) {}

    void AsyncLogger::FlushBuffers() {
        std::shared_lock guard(lock);
        for (U64 _offset{}; _offset < fmtlists.size() && count; count--) {
            const std::string_view line(&fmtlists[_offset], strchr(&fmtlists[_offset], '\n') + 1);
            outback->WriteType(line.data(), line.size(), _offset);
            _offset += line.size();
        }
        assert(count == 0);
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

            U32 count{};
            for (auto last{lists.rbegin()}; last != lists.rend() && count < 2; count++, ++last) {
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
