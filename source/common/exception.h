#pragma once
#include <fmt/format.h>

namespace FastNx {
    class exception final : public std::runtime_error {
    public:
        explicit exception(const std::string &errmsg) : std::runtime_error(errmsg) {}
        template<typename... Args>
        explicit exception(const fmt::format_string<Args...> &format, Args &&...args) : std::runtime_error(
            fmt::format(format, std::forward<Args>(args)...)) {}
    };
}
