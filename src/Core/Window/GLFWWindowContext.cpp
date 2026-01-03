#include "GlfwWindowContext.hpp"

#include <GLFW/glfw3.h>
#include <stdexcept>

GLFWWindowContext::GLFWWindowContext(uint32_t width, uint32_t height, const std::string &title, ResizeCallback callback) : userResizeCallback(callback) {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    glfwWindow = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    glfwSetWindowUserPointer(glfwWindow, this);
    glfwSetFramebufferSizeCallback(glfwWindow, staticFramebufferResizeCallback);
}

GLFWWindowContext::~GLFWWindowContext() {
    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
}

std::vector<const char*> GLFWWindowContext::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*> (glfwExtensions, glfwExtensions + glfwExtensionCount);
}

void GLFWWindowContext::createSurface(VkInstance instance, VkSurfaceKHR &surface) {
    if (glfwCreateWindowSurface(instance, glfwWindow, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

void GLFWWindowContext::waitEvents() {
    glfwWaitEvents();
}

bool GLFWWindowContext::shouldClose() {
    return glfwWindowShouldClose(glfwWindow);
}

void GLFWWindowContext::update() {
    glfwPollEvents();
}

void GLFWWindowContext::getFramebufferSize(uint32_t &width, uint32_t &height) {
    int c_width = 0, c_height = 0;
    glfwGetFramebufferSize(glfwWindow, &c_width, &c_height);
    width = static_cast<uint32_t>(c_width);
    height = static_cast<uint32_t>(c_height);
}

void GLFWWindowContext::staticFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    GLFWWindowContext* self = reinterpret_cast<GLFWWindowContext*>(glfwGetWindowUserPointer(window));
    self->onResize(width, height);
}

void GLFWWindowContext::onResize(int width, int height) {
    userResizeCallback(width, height);
}