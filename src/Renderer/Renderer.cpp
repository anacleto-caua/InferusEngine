#include "Renderer.hpp"
#include "Core/Window.hpp"

void Renderer::init(
    // Vulkan Context
    Window &window,
    const std::string &appName,
    const std::string &engineName,
    const std::vector<const char*> &instanceExtensions,
    const std::vector<const char*> &deviceExtensions,
    const std::vector<const char*> &validationLayers,
    const std::vector<const char*> &validationLayersExts
    // Renderer
    // ...
) {
    vulkanContext.init(
        window,
        appName,
        engineName,
        instanceExtensions,
        deviceExtensions,
        validationLayers,
        validationLayersExts
    );

    swapchain.init(vulkanContext, window, MAX_FRAMES_IN_FLIGHT);

}

Renderer::~Renderer() {

}
