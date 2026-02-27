// One must be carefull since this class utility is tied to the glfw
// library that has it's context initializated at the Window class

#pragma once

#include <array>
#include <functional>

#include <GLFW/glfw3.h>

#include "Engine/Types.hpp"
#include "Engine/Core/Window.hpp"

namespace InputSystem {
    enum class ActionType {
        Press,
        Repeat,
        Release
    };

    enum class InfKey {
        Forward,
        Backward,
        Right,
        Left,
        Up,
        Down,
        Interact,
        Escape,
        Crtl,

        _KEY_COUNT_
    };

    static constexpr size_t KEY_COUNT = static_cast<size_t>(InfKey::_KEY_COUNT_);

    static constexpr std::array<int, KEY_COUNT>GlfwKeyMap = {
        GLFW_KEY_W,
        GLFW_KEY_S,
        GLFW_KEY_D,
        GLFW_KEY_A,
        GLFW_KEY_Q,
        GLFW_KEY_E,
        GLFW_KEY_F,
        GLFW_KEY_ESCAPE,
        GLFW_KEY_LEFT_CONTROL
    };

    using UserAction = std::function<void()>;

    inline bool IsActive = true;

    InferusResult Init(Window& Window);

    void RegisterCallback(ActionType ActionType, InfKey Key, UserAction Callback);

    // Whithout an ActionType the callback is set for every Action possible
    void RegisterCallback(InfKey Key, UserAction Callback);
};
