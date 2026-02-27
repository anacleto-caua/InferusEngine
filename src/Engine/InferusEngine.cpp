#include "InferusEngine.hpp"

#include <cstdint>

#include <spdlog/spdlog.h>

#include "Engine/Core/InputSystem.hpp"

InferusResult InferusEngine::Init(){
    PlayerPos = { 0, 0, 0 };

    auto WindowResult = Window.Init(WIDTH, HEIGHT, ENGINE_NAME.data(), [this](uint32_t w, uint32_t h){this->Resize(w, h);});
    if (WindowResult != InferusResult::SUCCESS) {
        spdlog::error("Window creation failed.");
        return InferusResult::FAIL;
    }

    InputSystem::Init(Window);

    auto RendererResult = InferusRenderer.Init(Window);
    if (RendererResult != InferusResult::SUCCESS) {
        spdlog::error("Inferus Renderer creation failed.");
        return InferusResult::FAIL;
    }

    TerrainSystem.Init(&PlayerPos);
    InferusRenderer
        .TerrainRenderer.
        FullFeedTerrainData(
                InferusRenderer,
                TerrainSystem
                );
    Camera.Init(float(WIDTH)/float(HEIGHT), &InferusRenderer.TerrainRenderer.TerrainPushConstants.CameraMVP);

    InputSystem::RegisterCallback(InputSystem::ActionType::Press, InputSystem::InfKey::Forward, [](void){ spdlog::info("W as pressed"); });
    InputSystem::RegisterCallback(InputSystem::ActionType::Repeat, InputSystem::InfKey::Forward, [](void){ spdlog::info("W is being held"); });
    InputSystem::RegisterCallback(InputSystem::ActionType::Release, InputSystem::InfKey::Forward, [](void){ spdlog::info("W was released"); });

    return InferusResult::SUCCESS;
}

InferusEngine::~InferusEngine() {
    // ...
}

void InferusEngine::Run() {
    while (!ShouldClose && !Window.ShouldClose()) {
        InferusRenderer.EarlyRender();
        Window.Update();
        TerrainSystem.Update();
        InferusRenderer.LateRender();
    }
    Window.WaitEvents();
}

void InferusEngine::Resize(uint32_t Width, uint32_t Height) {
    InferusRenderer.Resize(Width, Height);
    Camera.Resize(float(Width)/float(Height));
}
