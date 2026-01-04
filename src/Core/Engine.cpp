#include "Engine.hpp"

#include <cstdint>

Engine::Engine() {

}

void Engine::framebufferResizeCallback(uint32_t width, uint32_t height) {

}

void Engine::init(std::string const title, uint32_t const width, uint32_t const height) {
    window.init(width, height, title, [this](uint32_t w, uint32_t h) { framebufferResizeCallback(w,  h); });
}

void Engine::run() {
    // --- setup ---

    mainLoop();
    
    // --- close up ---
}

void Engine::mainLoop() {
    while (!shouldClose()) {
        update();
        render();
    }
}

void Engine::update() {
    window.update();
}

void Engine::render() {}

bool Engine::shouldClose() {
    return window.shouldClose() || this->engineShouldClose;
}

void Engine::close() {
    engineShouldClose = true;
}