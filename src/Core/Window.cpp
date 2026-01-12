#include "Window.hpp"

#include <stdexcept>

#include <GLFW/glfw3.h>

void Window::init(uint32_t width, uint32_t height, const std::string &title, ResizeCallback callback) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(glfwWindow, this);

    userResizeCallback = callback;
    glfwSetFramebufferSizeCallback(glfwWindow, staticFramebufferResizeCallback);
}

Window::~Window() {
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

std::vector<const char*> Window::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*> (glfwExtensions, glfwExtensions + glfwExtensionCount);
}

void Window::createSurface(VkInstance instance, VkSurfaceKHR &surface) {
    if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void Window::waitEvents() {
    glfwWaitEvents();
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(glfwWindow);
}

void Window::update() {
    glfwPollEvents();
}

void Window::getFramebufferSize(uint32_t &width, uint32_t &height) {
    int c_width = 0, c_height = 0;
    glfwGetFramebufferSize(glfwWindow, &c_width, &c_height);
    width = static_cast<uint32_t>(c_width);
    height = static_cast<uint32_t>(c_height);
}

void Window::staticFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    Window* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    self->onResize(width, height);
}

void Window::onResize(uint32_t width, uint32_t height) {
    userResizeCallback(width, height);
}