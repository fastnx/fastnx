#pragma once
#include <core/assets.h>

namespace FastNx::Core {
    class Application {
    public:
        Application();
        ~Application();
        void Initialize();

    private:
        std::shared_ptr<Assets> amApp;
    };
}
