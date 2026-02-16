#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "Apps/MeshApp/Components/TerrainConfig.hpp"

namespace HeightmapConfig {
    constexpr VkFormat IMAGE_FORMAT = VK_FORMAT_R16_UNORM;
    constexpr size_t HEIGHTMAP_SIZE = TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION * sizeof(uint16_t) * TerrainConfig::INSTANCE_COUNT;
};
