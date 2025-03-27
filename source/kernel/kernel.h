#pragma once
#include <map>
#include <mutex>

#include <kernel/dram/device_lpddr4.h>
#include <kernel/types/kprocess.h>

namespace FastNx::Kernel {
    class KAutoObject;

    constexpr auto InitialProcessId{0x51};
    constexpr auto MaximumProcessIds{300};

    class Kernel {
    public:
        Kernel();
        I32 GetPid(const Types::KProcess &process);
        std::map<U64, KAutoObject*> autorefs;
    private:
        I32 pidseed{InitialProcessId};
        std::map<I32, std::array<U64, 4>> pidslist;


        Dram::DeviceLppd4 virtmem;
        std::mutex idsMutex;
    };
}
