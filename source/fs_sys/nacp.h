#pragma once
#include <fs_sys/types.h>

namespace FastNx::FsSys {
#pragma pack(push, 1)
    struct ApplicationTitle {
        std::array<char, 0x200> title;
        std::array<char, 0x100> publisher;
    };

    struct NacpHeader {
        std::array<ApplicationTitle, 0x10> apptitles;
    };
#pragma pack(pop)

    class Nacp {
    public:
        explicit Nacp(const VfsBackingFilePtr &nacp);
    };
}
