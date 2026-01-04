#pragma once

#include "Window.hpp"
#include "RHI/VulkanContext.hpp"

class Engine {
public:
private:
    const std::vector<const char*> INSTANCE_EXTENSIONS = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
    const std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME  };

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
