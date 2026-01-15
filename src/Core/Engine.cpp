#include "Engine.hpp"

#include <string>
#include <vector>
#include <cstdint>

void Engine::init(const std::string &appName) {
    uint32_t const WIDTH = 1200;
    uint32_t const HEIGHT = 800;

    std::string const ENGINE_NAME = "Inferus Engine";

    std::vector<const char*> INSTANCE_EXTENSIONS = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
    std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME };

    std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> VALIDATION_LAYERS_EXTENSION = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

    std::vector<const char*> VMA_SUGGESTED_EXTENSIONS = { VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME };

    std::string windowTitle = appName + " ~ Powered by: " + ENGINE_NAME;
    window.init(WIDTH, HEIGHT, appName, [this](uint32_t w, uint32_t h) { renderer.swapchain.resizeCallback(w, h); });

    std::vector<const char *> windowRequiredExtension = window.getRequiredExtensions();
    std::vector<const char *> finalInstanceExtensions;
    finalInstanceExtensions.insert(finalInstanceExtensions.end(), INSTANCE_EXTENSIONS.begin(), INSTANCE_EXTENSIONS.end());
    finalInstanceExtensions.insert(finalInstanceExtensions.end(), windowRequiredExtension.begin(), windowRequiredExtension.end());

    std::vector<const char*> finalDeviceExtensions;
    finalDeviceExtensions.insert(finalDeviceExtensions.end(), DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());
    finalDeviceExtensions.insert(finalDeviceExtensions.end(), VMA_SUGGESTED_EXTENSIONS.begin(), VMA_SUGGESTED_EXTENSIONS.end());

    renderer.init(window, appName, ENGINE_NAME, finalInstanceExtensions, finalDeviceExtensions, VALIDATION_LAYERS, VALIDATION_LAYERS_EXTENSION);
}

void Engine::run() {

}

void Engine::update() {
    window.update();
}

void Engine::render() {}

bool Engine::shouldClose() {
    return window.shouldClose() || this->engineShouldClose;
}

void Engine::close() {
    engineShouldClose = true;
}

Engine::~Engine() {
    vkDeviceWaitIdle(renderer.vulkanContext.device);
}