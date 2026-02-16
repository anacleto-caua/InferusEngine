#include "InferusEngine.hpp"
#include <cstdint>

InferusEngine::InferusEngine() : Window(WIDTH, HEIGHT, "Inferus Engine", this->Resize) {
}

InferusEngine::~InferusEngine() {
    // ...
}

void InferusEngine::Run() {
    while (!ShouldClose && !Window.ShouldClose()) {
        Window.Update();
        // ...
    }
    Window.WaitEvents();
}

void InferusEngine::Resize(uint32_t Width, uint32_t Height) {
    (void)Width;
    (void)Height;
}
