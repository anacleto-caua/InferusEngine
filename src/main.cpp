#include <iostream>
#include <exception>

#include "Core/Engine.hpp"

int main() {
    // TODO: Temporary config
    std::string const title = "Inferus Engine";
    uint32_t const width = 1200;
    uint32_t const height = 800;

    Engine app = Engine();
    app.init(title, width, height);

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return -1;
    }

    return 0;
}
