#pragma once

#include <glm/ext.hpp>

enum InferusResult {
    SUCCESS,
    FAIL
};

struct Camera {
    glm::mat4 MVP;
    float FOV;
};

