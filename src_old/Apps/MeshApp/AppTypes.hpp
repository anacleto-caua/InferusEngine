#pragma once

#include <glm/glm.hpp>

struct PushConstants {
    glm::mat4 mvp;
    glm::vec4 data;
};
