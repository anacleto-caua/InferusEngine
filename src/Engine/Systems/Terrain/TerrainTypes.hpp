#pragma once

#include <cstdint>

#include "glm/ext/vector_int2.hpp"

struct ChunkHeightmapLink {
    glm::ivec2 WorldPos;
    uint32_t InstanceId;
    uint16_t IsVisible;
};
