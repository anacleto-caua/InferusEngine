#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

class InferusRenderer;

#include "Engine/Types.hpp"
#include "Engine/Core/Window.hpp"

class ImGuiRenderer {
public:
    ImGuiRenderer() = default;
    ~ImGuiRenderer() = default;
    ImGuiRenderer(const ImGuiRenderer&) = delete;
    ImGuiRenderer& operator=(const ImGuiRenderer&) = delete;

    InferusResult Init(Window &Window, InferusRenderer &InferusRenderer);
    void Destroy();

    void EarlyRender();
    void LateRender(VkCommandBuffer cmd);

};
