#pragma once
#include <fmt/format.h>

namespace FastNx {
    class exception final : public std::runtime_error {
    public:
        template<typename... Args>
        explicit exception(const fmt::format_string<Args...> &format, Args &&...args) : std::runtime_error(
            fmt::format(format, std::forward<Args>(args)...)) {}
    };
}
