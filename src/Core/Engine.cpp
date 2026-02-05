#include "Engine.hpp"

#include <glm/fwd.hpp>
#include <string>
#include <vector>
#include <cstdint>

void Engine::init(const std::string &appName, glm::mat4* pMvp) {
    uint32_t const WIDTH = 1200;
    uint32_t const HEIGHT = 800;

    std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        // VMA extensions
        VK_EXT_MEMORY_BUDGET_EXTENSION_NAME, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME
    };

    std::vector<const char*> INSTANCE_EXTENSIONS = { VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME };

    std::string windowTitle = appName + " ~ Powered by: " + ENGINE_NAME;
    window.init(WIDTH, HEIGHT, windowTitle, [this](uint32_t w, uint32_t h) { renderer.resizeCallback(w, h); });

    std::vector<const char *> finalInstanceExtensions = window.getRequiredExtensions();
    finalInstanceExtensions.insert(finalInstanceExtensions.end(), INSTANCE_EXTENSIONS.begin(), INSTANCE_EXTENSIONS.end());

    renderer.init(window, appName, ENGINE_NAME, finalInstanceExtensions, DEVICE_EXTENSIONS, pMvp);
}

Engine::~Engine() {}

void Engine::update() {
    window.update();
}

void Engine::render(std::function<void(VkCommandBuffer)> drawCallback) {
    VkCommandBuffer cmd = renderer.beginFrame();

    if (cmd) {
        drawCallback(cmd);
        renderer.endFrame();
    }
}

bool Engine::shouldClose() {
    return window.shouldClose() || this->engineShouldClose;
}

void Engine::close() {
    engineShouldClose = true;
}
