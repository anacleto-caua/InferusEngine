#include <exception>

#include <spdlog/spdlog.h>

#include "Scenes/TestApp.hpp"

int main() {
    // [Time] [Log Level] Message
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    #ifdef NDEBUG
        spdlog::set_level(spdlog::level::off);
    #else
        spdlog::set_level(spdlog::level::debug);
    #endif

    TestApp app = TestApp();
    app.init();

    try {
        app.run();
    } catch (const std::exception &e) {
        spdlog::critical("runtime exception - ", e.what());
        return -1;
    }

    return 0;
}
