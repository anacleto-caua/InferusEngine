#include "Window.hpp"

#include <GLFW/glfw3.h>

Window::Window(uint32_t Width, uint32_t Height, const std::string &Title, ResizeCallback Callback) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    glfwWindow = glfwCreateWindow(Width, Height, Title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(glfwWindow, this);

    userResizeCallback = Callback;
    glfwSetFramebufferSizeCallback(glfwWindow, StaticFramebufferResizeCallback);
}

Window::~Window() {
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

std::vector<const char*> Window::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*> (glfwExtensions, glfwExtensions + glfwExtensionCount);
}

VkResult Window::CreateSurface(VkInstance instance, VkSurfaceKHR &surface) {
    return glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface);
}

void Window::WaitEvents() {
    glfwWaitEvents();
}

bool Window::ShouldClose() {
    return glfwWindowShouldClose(glfwWindow);
}

void Window::Update() {
    glfwPollEvents();
}

void Window::GetFramebufferSize(uint32_t &Width, uint32_t &Height) {
    int c_width = 0, c_height = 0;
    glfwGetFramebufferSize(glfwWindow, &c_width, &c_height);
    Width = static_cast<uint32_t>(c_width);
    Height = static_cast<uint32_t>(c_height);
}

void Window::StaticFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    self->OnResize(width, height);
}

void Window::OnResize(uint32_t width, uint32_t height) {
    userResizeCallback(width, height);
}
