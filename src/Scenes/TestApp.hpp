#pragma once

#include "Core/Engine.hpp"

class TestApp {
public:
private:
    Engine engine;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
public:
    TestApp() = default;
    ~TestApp();
    TestApp(const TestApp&) = delete;
    TestApp& operator=(const TestApp&) = delete;

    void init();

    void run();

    bool shouldClose();

    static void drawCallback(VkCommandBuffer commandBuffer, TestApp* app);
private:
};