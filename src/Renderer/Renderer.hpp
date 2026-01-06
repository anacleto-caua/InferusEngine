#include "RHI/VulkanContext.hpp"

struct FrameData {
    float deltaTime;
    // ...
};

class Renderer {
public:
    VulkanContext vulkanContext;
private:

public:
    Renderer() = default;
    ~Renderer();

    void init(
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
    );
private:
};