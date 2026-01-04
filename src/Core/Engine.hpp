#pragma once

#include "Window.hpp"

class Engine {
public:
private:
    Window window;
    bool engineShouldClose = false;

public:
    Engine();
    ~Engine() = default;

    void run();
    
    void init(std::string const title, uint32_t const width, uint32_t const height);
    void mainLoop();
    
    void update();
    void render();

    bool shouldClose();
    void close();
    
private:
    void framebufferResizeCallback(uint32_t width, uint32_t height);
};
