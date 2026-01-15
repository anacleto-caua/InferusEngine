#pragma once

#include "Core/Window.hpp"
#include "Renderer/Renderer.hpp"

class Engine {
public:
private:
    bool engineShouldClose = false;

    Window window;
    Renderer renderer;

public:
    Engine() = default;
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;


    void init(std::string const &appName, std::string const &engineName, uint32_t const width, uint32_t const height);

    void run();

    void update();
    void render();

    bool shouldClose();
    void close();

private:
};
