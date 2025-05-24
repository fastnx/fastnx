#pragma once

#include <core/assets.h>
#include <core/app_setup.h>
#include <horizon/switch_ns.h>
#include <horizon/key_set.h>


namespace FastNx::Core {
    class Application {
    public:
        Application();
        ~Application();
        void Initialize();

        void DumpAllLogos() const;

        void LoadFirstPickedGame() const;

        std::shared_ptr<AppSetup> settings;
    private:
        std::shared_ptr<Assets> assets;
        std::shared_ptr<Horizon::KeySet> keys;
        std::shared_ptr<Horizon::SwitchNs> switchnx;
    };

    std::shared_ptr<Application> GetContext();
}
