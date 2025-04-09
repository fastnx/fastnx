#pragma once

#include <shared_mutex>
#include <source_location>
#include <fmt/format.h>

#include <fs_sys/types.h>
#include <common/types.h>

#include <fs_sys/refs/editable_directory.h>

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
        explicit AsyncLogger(std::ostream &output);
        explicit AsyncLogger(const FsSys::VfsBackingFilePtr &file);

        void FlushBuffers();
        template<typename ...Args>
        static void Puts(const fmt::format_string<Args...> &fmt, Args &&...args) {
            logger->Log(fmt::format(fmt, std::forward<Args>(args)...));
        }
        static void Puts(const std::string &format) {
            logger->Puts("{}", format);
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

        U64 threshold{32};
        U64 count{};
        fmt::memory_buffer fmtlists; // We should prioritize using our memory instead of calling I/O functions
    private:
        void Log(LogType _type, const std::source_location &source, std::string &&fmtmsg);
        void Log(std::string &&fmtmsg);

        FsSys::VfsBackingFilePtr outback;
        std::shared_mutex lock;
    };

    std::shared_ptr<AsyncLogger> BuildAsyncLogger(FsSys::ReFs::EditableDirectory *logdir = {});
}
