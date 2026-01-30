#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace TerrainConfig {
    struct ChunkData {
        glm::ivec2 worldPos;
        uint32_t instanceId;
        uint32_t isVisible;
    };

    constexpr uint32_t INSTANCE_COUNT = 9;
    constexpr uint32_t SQRT_INSTANCE_COUNT = 3;
    constexpr uint32_t INDICES_COUNT = 64;
};
