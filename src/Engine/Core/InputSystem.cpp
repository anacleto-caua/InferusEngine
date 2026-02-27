#include "InputSystem.hpp"

#include <vector>

#include "spdlog/spdlog.h"

namespace InputSystem {
    std::vector<UserAction> PressActions;
    std::vector<UserAction> RepeatActions;
    std::vector<UserAction> ReleaseActions;

    void KeyCallbackStrain([[maybe_unused]]GLFWwindow* window, int key, [[maybe_unused]]int scancode, int action, [[maybe_unused]]int mods) {
        if (action == GLFW_PRESS) {
            if (static_cast<size_t>(key) < PressActions.size()) {
                if (PressActions[key]) {
                    PressActions[key]();
                }
            }
            return;
        }
        if (action == GLFW_REPEAT) {
            if (static_cast<size_t>(key) < RepeatActions.size()) {
                if (RepeatActions[key]) {
                    RepeatActions[key]();
                }
            }
            return;
        }
        if (action == GLFW_RELEASE) {
            if (static_cast<size_t>(key) < ReleaseActions.size()) {
                if (ReleaseActions[key]) {
                    ReleaseActions[key]();
                }
            }
            return;
        }
    }

    InferusResult Init(Window& Window) {
        glfwSetKeyCallback(Window.glfwWindow, KeyCallbackStrain);
        return InferusResult::SUCCESS;
    }

    void RegisterCallback(ActionType ActionType, InfKey Key, UserAction Callback) {

        auto PushKeyAction = [&Callback, &Key](std::vector<UserAction>& vec) {
            size_t KeyIndex = static_cast<size_t>(GlfwKeyMap[static_cast<size_t>(Key)]);
            if (vec.size() < KeyIndex) {
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
