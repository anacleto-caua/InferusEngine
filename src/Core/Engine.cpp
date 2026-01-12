#include "Engine.hpp"

#include <cstdint>
#include <vector>

void Engine::init(const std::string &appName, std::string const &engineName, uint32_t const width, uint32_t const height) {
    std::vector<const char*> INSTANCE_EXTENSIONS = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };
    std::vector<const char*> DEVICE_EXTENSIONS = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME  };

    std::vector<const char*> VALIDATION_LAYERS = { "VK_LAYER_KHRONOS_validation" };
    std::vector<const char*> VALIDATION_LAYERS_EXTENSION = { "VK_EXT_debug_utils" };

    window.init(width, height, appName, [this](uint32_t w, uint32_t h) { renderer.swapchain.resizeCallback(w, h); });

    std::vector<const char *> windowRequiredExtension = window.getRequiredExtensions();
    std::vector<const char *> instanceExtensions;
    instanceExtensions.insert(instanceExtensions.end(), windowRequiredExtension.begin(), windowRequiredExtension.end());
    instanceExtensions.insert(instanceExtensions.end(), INSTANCE_EXTENSIONS.begin(), INSTANCE_EXTENSIONS.end());

    renderer.init(window, appName, engineName, instanceExtensions, DEVICE_EXTENSIONS, VALIDATION_LAYERS, VALIDATION_LAYERS_EXTENSION);
}

void Engine::run() {
    while (!shouldClose()) {
        update();
        render();
    }
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