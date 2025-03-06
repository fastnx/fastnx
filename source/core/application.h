#pragma once

#include <core/assets.h>
#include <horizon/switch_ns.h>
#include <horizon/key_set.h>
namespace FastNx::Core {
    class Application {
    public:
        Application();
        ~Application();
        void Initialize();

        void LoadFirstPickedGame() const;
    private:
        std::shared_ptr<Assets> assets;
        std::shared_ptr<Horizon::KeySet> keys;
        std::shared_ptr<Horizon::SwitchNs> _switch;
    };
}
