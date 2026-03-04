// One must be carefull since this class utility is tied to the glfw
// library that has it's context initializated at the Window class

#pragma once

#include <functional>

#include <GLFW/glfw3.h>

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
        Ctrl,

        _KEY_COUNT_
    };

    using UserAction = std::function<void()>;

    inline bool IsActive = true;

    void Create();
    void Destroy();

    void PollInput();

    void RegisterCallback(ActionType ActionType, InfKey Key, UserAction Callback);
    void RegisterCallback(InfKey Key, UserAction Callback);
};
