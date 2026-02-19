#include "InferusEngine.hpp"

#include <cstdint>

InferusEngine::InferusEngine()
    : Window(WIDTH, HEIGHT, "Inferus Engine", [this](uint32_t w, uint32_t h){this->Resize(w, h);}),
    InferusRenderer(Window){
}

InferusEngine::~InferusEngine() {
    // ...
}

void InferusEngine::Run() {
    while (!ShouldClose && !Window.ShouldClose()) {
        Window.Update();
    InferusRenderer.Render();
        // ...
    }
    Window.WaitEvents();
}

void InferusEngine::Resize(uint32_t Width, uint32_t Height) {
    InferusRenderer.Resize(Width, Height);
}
