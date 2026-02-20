#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "Engine/Components/Terrain/TerrainTypes.hpp"

namespace TerrainConfig {
    namespace Chunk {
        constexpr uint32_t RESOLUTION = 64;

        constexpr uint32_t INDICES_COUNT = (RESOLUTION - 1) * (RESOLUTION - 1) * 6;

        constexpr uint32_t INDICES_BUFFER_SIZE = INDICES_COUNT * sizeof(uint32_t);
    };

    namespace ChunkToHeighmatLinking {
        constexpr uint32_t DIAMOND_EXPLORATION_RADIUS = 4;

        constexpr uint32_t INSTANCE_COUNT = []{
            constexpr uint32_t X = DIAMOND_EXPLORATION_RADIUS;
            return (X * X) + ((X + 1) * (X + 1));
        }();

        constexpr uint32_t LINKING_BUFFER_SIZE = INSTANCE_COUNT * sizeof(ChunkHeightmapLink);
    };

    namespace Heightmap {
        constexpr VkFormat HEIGHTMAP_IMAGE_FORMAT = VK_FORMAT_R16_UNORM;

        constexpr size_t HEIGHTMAP_IMAGE_SIZE = TerrainConfig::Chunk::RESOLUTION * TerrainConfig::Chunk::RESOLUTION * sizeof(uint16_t) * TerrainConfig::ChunkToHeighmatLinking::INSTANCE_COUNT;
    };
};
