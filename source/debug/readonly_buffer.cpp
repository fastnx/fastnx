#include <fmt/ranges.h>

#include <common/async_logger.h>
#include <debug/readonly_buffer.h>


namespace FastNx::Debug {
    // https://linux.die.net/man/1/strings
    // Dumps all readable content up to N strings from the specified address
    fmt::memory_buffer Strings(const void *begin, const U64 size, const U64 strings) {
        fmt::memory_buffer result;
        constexpr auto MinimumStringSize{3};
        // Assuming that 1/3 of begin consists of plaintext
        result.reserve(std::min(size / 3, strings * MinimumStringSize * 12));
        U64 recstrs{};

        const std::string_view redable{static_cast<const char *>(begin), size};
        auto redableit{redable.begin()};

        for (; redableit != redable.end(); ++redableit) {
            if (!std::isprint(*redableit))
                continue;
            U64 counter{};
            while (std::isprint(*++redableit))
                counter++;
            if (!counter || counter < MinimumStringSize)
                continue;

            redableit -= counter;
            fmt::format_to(std::back_inserter(result), "{} ", redable.substr(std::max(std::distance(redable.begin(), redableit) - 1, 0L), counter + 1));
            redableit += counter;

            if (recstrs++ == strings)
                break;
        }
        return result;
    }

    void PrintTopics(const std::string_view &strings, const FsSys::FsPath &nsoname) {
        U64 found{};
        auto SearchForTopics = [&](const std::initializer_list<const char *> topics, const char *message) {
            std::vector<const char *> matches;

            for (const auto *topic: topics)
                if (strings.contains(topic))
                    matches.emplace_back(topic);
            if (!matches.empty())
                AsyncLogger::Puts("Topics({} - {}) ", fmt::join(matches, ", "), message);
            found += matches.size();
        };
        AsyncLogger::Puts("Searching for topics in NSO {}: ", FsSys::GetPathStr(nsoname));

        SearchForTopics({"android"}, "Some symbol or feature for the Android platform found");
        SearchForTopics({"musl"}, "musl libc library found");
        SearchForTopics({"memcpy", "memmove", "putc", "printf"}, "C library found");

        if (found)
            AsyncLogger::Puts("\n");
        else
            AsyncLogger::ClearLine();
    }
}
