#pragma once

#include <shared_mutex>
#include <source_location>
#include <fmt/format.h>

#include <fs_sys/types.h>
#include <common/types.h>

#include <fs_sys/refs/directory_file_access.h>

namespace FastNx {
    enum class LogType {
        Success,
        Info,
        Error
    };

    class AsyncLogger;
    inline std::shared_ptr<AsyncLogger> logger;

    struct FormatWrapper {
        template<typename T>
        // ReSharper disable once CppNonExplicitConvertingConstructor
        FormatWrapper(const T &string, const std::source_location &_source = std::source_location::current()) : format(string), location(_source) {}
        std::string_view format;
        std::source_location location;
    };

    class AsyncLogger {
    public:
        AsyncLogger() = delete;
        explicit AsyncLogger(std::ostream &output, U64 size = 32);
        explicit AsyncLogger(const FsSys::VfsBackingFilePtr &file, U64 size = 32);

        void FlushBuffers();
        template<typename ...Args>
        static void Puts(const fmt::format_string<Args...> &fmt, Args &&...args) {
            logger->Log(fmt::format(fmt, std::forward<Args>(args)...));
        }
        static void Puts(const std::string &format) {
            logger->Puts("{}", format);
        }
        static void ClearLine() {
            const std::string_view logs{logger->fmtlists.data(), logger->fmtlists.size()};
            if (const auto length{logs.find_last_of('\n')}; length != std::string::npos)
                logger->fmtlists.resize(length + 1);
        }

        template<typename ...Args>
        static void Success(const FormatWrapper &fmt, Args &&...args) {
            logger->Log(LogType::Success, fmt.location, fmt::format(fmt::runtime(fmt.format), std::forward<Args>(args)...));
        }
        template<typename ...Args>
        static void Info(const FormatWrapper &fmt, Args &&...args) {
            logger->Log(LogType::Info, fmt.location, fmt::format(fmt::runtime(fmt.format), std::forward<Args>(args)...));
        }
        template<typename ...Args>
        static void Error(const FormatWrapper &fmt, Args &&...args) {
            logger->Log(LogType::Error, fmt.location, fmt::format(fmt::runtime(fmt.format), std::forward<Args>(args)...));
        }

        U64 threshold;
        U64 count{};
        fmt::memory_buffer fmtlists; // We should prioritize using our memory instead of calling I/O functions
    private:
        void Log(LogType _type, const std::source_location &source, std::string &&fmtmsg);
        void Log(std::string &&fmtmsg);

        FsSys::VfsBackingFilePtr backing;
        std::shared_mutex lock;
    };

    std::shared_ptr<AsyncLogger> BuildAsyncLogger(FsSys::ReFs::DirectoryFileAccess *logdir = {});
}
