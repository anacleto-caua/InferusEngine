#include "InferusEngine.hpp"
#include <cstdint>

InferusEngine::InferusEngine()
    : Window(WIDTH, HEIGHT, "Inferus Engine", [this](uint32_t w, uint32_t h){this->Resize(w, h);}),
    Renderer(Window){
}

InferusEngine::~InferusEngine() {
    // ...
}

void InferusEngine::Run() {
    while (!ShouldClose && !Window.ShouldClose()) {
        Window.Update();
        Renderer.Render();
        // ...
    }
    Window.WaitEvents();
}

void InferusEngine::Resize(uint32_t Width, uint32_t Height) {
    Renderer.Resize(Width, Height);
}
