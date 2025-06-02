#pragma once

#include <map>
#include <common/types.h>

#include <kernel/types/kprocess.h>

namespace FastNx::Debug {
    class ProcessCalltracer {
    public:
        ProcessCalltracer() = default;

        explicit ProcessCalltracer(const std::shared_ptr<Kernel::Types::KProcess> &process);

    private:
        void GetMod0(const U8 *begin);
        std::map<U64, std::pair<U8, std::string>> solvedsyms;
    };
}
