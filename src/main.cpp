#include <exception>

#include <spdlog/spdlog.h>

#include "Core/Engine.hpp"

int main() {
    // TODO: Temporary config
    std::string const APP_NAME = "Inferus Engine";
    std::string const ENGINE_NAME = "Inferus Engine";
    uint32_t const WIDTH = 1200;
    uint32_t const HEIGHT = 800;

    // [Time] [Log Level] Message
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");
    #ifdef NDEBUG
        spdlog::set_level(spdlog::level::off);
    #else
        spdlog::set_level(spdlog::level::debug);
    #endif

    Engine app = Engine();
    app.init(APP_NAME, ENGINE_NAME, WIDTH, HEIGHT);

    try {
        app.run();
    } catch (const std::exception &e) {
        spdlog::critical("runtime exception - ", e.what());
        return -1;
    }

    return 0;
}
