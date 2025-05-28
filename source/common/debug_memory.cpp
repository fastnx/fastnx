#include <common/debug_memory.h>
namespace FastNx {
    // https://linux.die.net/man/1/strings
    // Dumps all readable content up to N strings from the specified address
    fmt::memory_buffer Strings(const void *begin, const U64 size, const U64 strings) {
        fmt::memory_buffer result;
        result.reserve(size / 3); // Assuming that 1/3 of begin consists of plaintext
        U64 recstrs{};

        const std::string_view redable{static_cast<const char *>(begin), size};
        auto redableit{redable.begin()};
        constexpr auto MinimumStringSize{3};

        for (; redableit != redable.end(); ++redableit) {
            if (!std::isprint(*redableit))
                continue;
            U64 counter{};
            while (std::isprint(*++redableit))
                counter++;
            if (!counter || counter < MinimumStringSize)
                continue;

            redableit -= counter;
            fmt::format_to(std::back_inserter(result), "{} ", redable.substr(std::distance(redable.begin(), redableit) - 1, counter + 1));
            redableit += counter;

            if (recstrs++ == strings)
                break;
        }
        return result;
    }
}
