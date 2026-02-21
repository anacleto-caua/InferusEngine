#pragma once

#include <string_view>

#include "Engine/Types.hpp"
#include "Engine/Components/Window.hpp"
#include "Engine/InferusRenderer/InferusRenderer.hpp"
#include "Engine/Components/Terrain/TerrainSystem.hpp"

class InferusEngine {
public:
    static constexpr std::string_view ENGINE_NAME = "Inferus Engine";
    static constexpr uint32_t WIDTH = 1280;
    static constexpr uint32_t HEIGHT = 720;

    bool ShouldClose = false;

    Window Window;
    InferusRenderer InferusRenderer;
    TerrainSystem TerrainSystem;
private:
    glm::ivec3 PlayerPos;
public:
    InferusEngine() = default;
    ~InferusEngine();
    InferusEngine(const InferusEngine&) = delete;
    InferusEngine& operator=(const InferusEngine&) = delete;

    InferusResult Init();

    void Run();
private:
    void Resize(uint32_t Width, uint32_t Height);
};
