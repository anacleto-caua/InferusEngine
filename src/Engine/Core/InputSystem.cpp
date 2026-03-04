#include "InputSystem.hpp"

#include <array>
#include <vector>

#include <spdlog/spdlog.h>

#include "Engine/Core/Window.hpp"

namespace InputSystem {
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

    struct KeyState {
        bool IsPressed = false;
    };
    std::vector<KeyState> KeyStates {};

    std::vector<UserAction> PressActions {};
    std::vector<UserAction> ReleaseActions {};

    std::vector<UserAction> RepeatActions {};
    std::vector<size_t> PollingKeys {}; // It's carries glfw keys which map to the repeat actions array

    // Map to find the InfKey
    static const std::unordered_map<int, InfKey> ReverseKeyMap = []() {
        std::unordered_map<int, InfKey> map;
        for (size_t i = 0; i < KEY_COUNT; ++i) {
            map[GlfwKeyMap[i]] = static_cast<InfKey>(i);
        }
        return map;
    }();

    InfKey ReverseMapInfKey(int glfwKey) {
        return ReverseKeyMap.find(glfwKey)->second;
    }

    void KeyCallbackStrain(
            [[maybe_unused]]GLFWwindow* window,
            int key,
            [[maybe_unused]]int scancode,
            int action,
            [[maybe_unused]]int mods)
    {
        if (action == GLFW_PRESS) {
            if (static_cast<size_t>(key) < PressActions.size()) {
                if (PressActions[key]) {
                    PressActions[key]();
                }
                KeyStates[key].IsPressed = true;
            }
            return;
        }
        if (action == GLFW_RELEASE) {
            if (static_cast<size_t>(key) < ReleaseActions.size()) {
                if (ReleaseActions[key]) {
                    ReleaseActions[key]();
                }
                KeyStates[key].IsPressed = false;
            }
            return;
        }
    }

    void Create() {
        glfwSetKeyCallback(Window::glfwWindow, KeyCallbackStrain);
    }

    void Destroy() {
        // ...
    }

    void PollInput() {
        for(size_t glfwKey : PollingKeys) {
            if (KeyStates[glfwKey].IsPressed) {
                RepeatActions[glfwKey]();
            }
        }
    }

    void RegisterCallback(ActionType ActionType, InfKey Key, UserAction Callback) {
        size_t KeyIndex = static_cast<size_t>(GlfwKeyMap[static_cast<size_t>(Key)]);

        if (KeyIndex > KeyStates.size()) {
            KeyStates.resize(KeyIndex + 1);
        }

        auto PushKeyAction = [&KeyIndex, &Callback](std::vector<UserAction>& vec) {
            if (KeyIndex > vec.size()) {
                vec.resize(KeyIndex + 1);
            }

            if (vec[KeyIndex]) {
                spdlog::warn("Input action is being overwriten");
            }
            vec[KeyIndex] = Callback;
        };

        switch (ActionType) {
            case ActionType::Press:
                PushKeyAction(PressActions);
                break;
            case ActionType::Repeat:
                PushKeyAction(RepeatActions);
                PollingKeys.push_back(KeyIndex);
                break;
            case ActionType::Release:
                PushKeyAction(ReleaseActions);
                break;
        }
    }

    void RegisterCallback(InfKey Key, UserAction Callback) {
        RegisterCallback(ActionType::Press, Key, Callback);
        RegisterCallback(ActionType::Repeat, Key, Callback);
        RegisterCallback(ActionType::Release, Key, Callback);
    }

};
