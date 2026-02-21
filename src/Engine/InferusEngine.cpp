#include "InferusEngine.hpp"

#include <cstdint>

#include <spdlog/spdlog.h>

InferusResult InferusEngine::Init(){
    PlayerPos = { 0, 0, 0 };

    auto WindowResult = Window.Init(WIDTH, HEIGHT, ENGINE_NAME.data(), [this](uint32_t w, uint32_t h){this->Resize(w, h);});
    if (WindowResult != InferusResult::SUCCESS) {
        spdlog::error("Window creation failed.");
        return InferusResult::FAIL;
    }
    auto RendererResult = InferusRenderer.Init(Window);
    if (RendererResult != InferusResult::SUCCESS) {
        spdlog::error("Inferus Renderer creation failed.");
        return InferusResult::FAIL;
    }

    TerrainSystem.Init(&PlayerPos);

    return InferusResult::SUCCESS;
}

InferusEngine::~InferusEngine() {
    // ...
}

void InferusEngine::Run() {
    while (!ShouldClose && !Window.ShouldClose()) {
        Window.Update();
        InferusRenderer.Render();
        TerrainSystem.Update();
        // ...
    }
    Window.WaitEvents();
}

void InferusEngine::Resize(uint32_t Width, uint32_t Height) {
    InferusRenderer.Resize(Width, Height);
}
