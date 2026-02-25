#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include "Engine/Types.hpp"

using ResizeCallback = std::function<void(uint32_t Width, uint32_t Height)>;

class Window {
public:
    GLFWwindow *glfwWindow = nullptr;

private:
    ResizeCallback userResizeCallback = nullptr;

public:
    Window() = default;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    InferusResult Init(uint32_t Width, uint32_t Height, const std::string &Title, ResizeCallback OwnerCallback);

    std::vector<const char*> GetRequiredExtensions();
    VkResult CreateSurface(VkInstance instance, VkSurfaceKHR &surface);

    void GetFramebufferSize(uint32_t &width, uint32_t &height);

    void WaitEvents();
    void Update();
    bool ShouldClose();

private:
    static void StaticFramebufferResizeCallback(GLFWwindow* window, int width, int height);
    void OnResize(uint32_t width, uint32_t height);
};
