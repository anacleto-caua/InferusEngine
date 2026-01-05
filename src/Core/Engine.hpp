#pragma once

#include "Window.hpp"
#include "RHI/VulkanContext.hpp"

class Engine {
public:
private:
    bool engineShouldClose = false;

    Window window;
    VulkanContext vulkanContext;

public:
    Engine() = default;
    ~Engine() = default;

    void init(std::string const &appName, std::string const &engineName, uint32_t const width, uint32_t const height);
   
    void run();
    
    void update();
    void render();

    bool shouldClose();
    void close();

private:
    void framebufferResizeCallback(uint32_t width, uint32_t height);
};
