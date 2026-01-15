#include "TestApp.hpp"

void TestApp::init() {
    const std::string APP_NAME = "TestApp";
    engine.init(APP_NAME);
}

void TestApp::run() {
    while (!shouldClose()) {
        engine.update();
        engine.render();
    }
}

bool TestApp::shouldClose() {
    return engine.shouldClose();
}