#pragma once

#include "Core/Window.hpp"
#include "Renderer/Renderer.hpp"

class Engine {
public:
    Window window;
    Renderer renderer;
private:
    bool engineShouldClose = false;
public:
    Engine() = default;
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void init(std::string const &appName);

    void update();
    void render(std::function<void(VkCommandBuffer)> drawCallback);

    bool shouldClose();
    void close();

private:
};
