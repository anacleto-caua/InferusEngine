#pragma once

#include <string_view>

#include "Engine/Components/Window.hpp"
#include "Engine/InferusRenderer/InferusRenderer.hpp"

class InferusEngine {
public:
    static constexpr std::string_view ENGINE_NAME = "Inferus Engine";
    static constexpr uint32_t WIDTH = 1280;
    static constexpr uint32_t HEIGHT = 720;

    bool ShouldClose = false;

    Window Window;
    InferusRenderer InferusRenderer;
private:
public:
    InferusEngine();
    ~InferusEngine();
    InferusEngine(const InferusEngine&) = delete;
    InferusEngine& operator=(const InferusEngine&) = delete;

    void Run();
private:
    void Resize(uint32_t Width, uint32_t Height);
};
