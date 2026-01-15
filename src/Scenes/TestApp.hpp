#pragma once

#include "Core/Engine.hpp"

class TestApp {
public:
private:
    Engine engine;

public:
    TestApp() = default;
    ~TestApp() = default;
    TestApp(const TestApp&) = delete;
    TestApp& operator=(const TestApp&) = delete;

    void init();

    void run();

    bool shouldClose();
private:
};