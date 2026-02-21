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

    namespace ChunkToHeightmapLinking {
        constexpr uint32_t DIAMOND_EXPLORATION_RADIUS = 4;

        constexpr uint32_t INSTANCE_COUNT = []{
            constexpr uint32_t X = DIAMOND_EXPLORATION_RADIUS;
            return (X * X) + ((X + 1) * (X + 1));
        }();

        constexpr uint32_t LINKING_BUFFER_SIZE = INSTANCE_COUNT * sizeof(ChunkHeightmapLink);
    };

    namespace Heightmap {
        constexpr VkFormat HEIGHTMAP_IMAGE_FORMAT = VK_FORMAT_R16_UNORM;

        constexpr size_t HEIGHTMAP_IMAGE_PIXEL_COUNT = TerrainConfig::Chunk::RESOLUTION * TerrainConfig::Chunk::RESOLUTION;

        constexpr size_t HEIGHTMAP_ALL_IMAGES_PIXEL_COUNT = HEIGHTMAP_IMAGE_PIXEL_COUNT * TerrainConfig::ChunkToHeightmapLinking::INSTANCE_COUNT;

        constexpr size_t HEIGHTMAP_IMAGE_SIZE =  HEIGHTMAP_IMAGE_PIXEL_COUNT * sizeof(uint16_t);

        constexpr size_t HEIGHTMAP_ALL_IMAGES_SIZE =  HEIGHTMAP_ALL_IMAGES_PIXEL_COUNT * sizeof(uint16_t);
    };
};
