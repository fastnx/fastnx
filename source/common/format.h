#pragma once

#include <common/traits.h>

struct FormatSize {
    explicit FormatSize(const size_t size) : value(size) {}

    template<typename T>
    explicit FormatSize(const T &container) {
        if constexpr (FastNx::IsVectorType<T>)
            value = FastNx::ToSpan(container).size_bytes();
    }

    size_t value{};
};

template<>
struct fmt::formatter<FormatSize> {
    static constexpr auto parse(const auto &ctx) {
        return ctx.begin();
    }
    template<typename FormatContext>
    auto format(const FormatSize &size, FormatContext &ctx) const {
        static const char *units[] = {"B", "kB", "MB", "GB", nullptr};
        const char **fmt{units};
        auto value{static_cast<double>(size.value)};
        while (value > 1024) {
            value /= 1024;
            if (!++fmt)
                std::terminate();
        }
        return fmt::format_to(ctx.out(), "{} {}", value, *fmt);
    }
};
