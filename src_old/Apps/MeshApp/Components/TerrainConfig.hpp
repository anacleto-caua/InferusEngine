#pragma once

#include <cstdint>

#include <glm/glm.hpp>

namespace TerrainConfig {

    constexpr uint32_t DIAMOND_EXPLORATION_RADIUS = 4;

    constexpr uint32_t INSTANCE_COUNT = []{
        constexpr uint32_t X = DIAMOND_EXPLORATION_RADIUS;
        return (X * X) + ((X + 1) * (X + 1));
    }();

    constexpr uint32_t RESOLUTION = 64;
};
