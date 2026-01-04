#pragma  once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <vulkan/vulkan.h>

#include <GLFW/glfw3.h>

class Window {
public:
    GLFWwindow *glfwWindow = nullptr;

private:
    using ResizeCallback = std::function<void(uint32_t width, uint32_t height)>;
    ResizeCallback userResizeCallback = nullptr;

public:
    Window() = default;
    ~Window();

    void init(uint32_t width, uint32_t height, const std::string &title, ResizeCallback callback);

    std::vector<const char*> getRequiredExtensions();

    void createSurface(VkInstance instance, VkSurfaceKHR &surface);

    void getFramebufferSize(uint32_t &width, uint32_t &height);

    void waitEvents();

    void update();
    
    bool shouldClose();

private:
    static void staticFramebufferResizeCallback(GLFWwindow* window, int width, int height);
    void onResize(uint32_t width, uint32_t height);
};