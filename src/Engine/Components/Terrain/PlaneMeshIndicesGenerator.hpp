#pragma once

#include <cstdint>

#include <vulkan/vulkan.h>

#include "Engine/Components/Terrain/TerrainConfig.hpp"

namespace PlaneMeshIndicesGenerator {
    static inline void GetIndices(uint32_t* IndicesBegin) {
        int32_t TerrainRes = TerrainConfig::RESOLUTION;
        for (int z = 0; z < TerrainRes - 1; z++) {
            for (int x = 0; x < TerrainRes - 1; x++) {
                // Calculate the index of the current vertex and neighbors
                uint32_t topLeft = (z * TerrainConfig::RESOLUTION) + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = ((z + 1) * TerrainConfig::RESOLUTION) + x;
                uint32_t bottomRight = bottomLeft + 1;

                // Triangle 1 (Top-Left -> Bottom-Left -> Top-Right)
                *IndicesBegin++ = topLeft;
                *IndicesBegin++ = bottomLeft;
                *IndicesBegin++ = topRight;

                // Triangle 2 (Top-Right -> Bottom-Left -> Bottom-Right)
                *IndicesBegin++ = topRight;
                *IndicesBegin++ = bottomLeft;
                *IndicesBegin++ = bottomRight;
            }
        }
    }
}
