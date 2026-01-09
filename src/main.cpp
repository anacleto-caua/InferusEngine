#include <iostream>
#include <exception>

#include "Core/Engine.hpp"

int main() {
    // TODO: Temporary config
    std::string const APP_NAME = "Inferus Engine";
    std::string const ENGINE_NAME = "Inferus Engine";
    uint32_t const WIDTH = 1200;
    uint32_t const HEIGHT = 800;

    // [Time] [Log Level] Message
    spdlog::set_pattern("[%H:%M:%S] [%^%l%$] %v");

    Engine app = Engine();
    app.init(APP_NAME, ENGINE_NAME, WIDTH, HEIGHT);

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}
