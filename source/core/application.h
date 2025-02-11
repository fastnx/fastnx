#pragma once
#include <horizon/switch_ns.h>
#include <core/assets.h>

namespace FastNx::Core {
    class Application {
    public:
        Application();
        ~Application();
        void Initialize();

        void LoadFirstPickedGame() const;
    private:
        std::shared_ptr<Assets> filesAm;
        std::shared_ptr<Horizon::SwitchNs> _switch;
    };
}
