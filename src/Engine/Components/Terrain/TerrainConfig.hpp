#pragma once

#include <cstdint>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace TerrainConfig {

    constexpr uint32_t RESOLUTION = 64;

    constexpr uint32_t INDICES_COUNT = (TerrainConfig::RESOLUTION - 1) * (TerrainConfig::RESOLUTION - 1) * 6;

    constexpr uint32_t INDICES_BUFFER_SIZE = INDICES_COUNT * sizeof(uint32_t);

    /* Not used as one may notice
    constexpr uint32_t DIAMOND_EXPLORATION_RADIUS = 4;

    constexpr uint32_t INSTANCE_COUNT = []{
        constexpr uint32_t X = DIAMOND_EXPLORATION_RADIUS;
        return (X * X) + ((X + 1) * (X + 1));
    }();

    constexpr VkFormat IMAGE_FORMAT = VK_FORMAT_R16_UNORM;

    constexpr size_t HEIGHTMAP_SIZE = TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION * sizeof(uint16_t) * TerrainConfig::INSTANCE_COUNT;
    */
};
