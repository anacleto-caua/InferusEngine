#include <cmath>
#pragma once

#include <vector>
#include <cstdint>

#include "TerrainChunkData.hpp"

namespace NoiseGenerator {
    inline std::vector<uint16_t> generateMockHeightmaps() {
        // Total pixels = Width * Height * Layers
        std::vector<uint16_t> data(TerrainChunkData::RESOLUTION * TerrainChunkData::RESOLUTION * TerrainChunkData::INSTANCE_COUNT);

        for (int layer = 0; layer < TerrainChunkData::INSTANCE_COUNT; layer++) {
            // Calculate which grid cell this layer belongs to (0..2, 0..2)
            int gridX = layer % 3;
            int gridY = layer / 3;

            for (int y = 0; y < TerrainChunkData::RESOLUTION; y++) {
                for (int x = 0; x < TerrainChunkData::RESOLUTION; x++) {
                    // Global coordinates for noise
                    float globalX = (gridX * (TerrainChunkData::RESOLUTION - 1)) + x;
                    float globalY = (gridY * (TerrainChunkData::RESOLUTION - 1)) + y;

                    // Simple sine wave terrain
                    float height = std::sin(globalX * 0.1f) + std::cos(globalY * 0.1f);

                    // Normalize to 0..1 range roughly
                    height = (height * 0.25f) + 0.5f;

                    // Convert to 16-bit integer (0 to 65535)
                    uint16_t value = static_cast<uint16_t>(height * 65535.0f);

                    // Index the texture array
                    // [Layer][Row][Col]
                    int index = (layer * TerrainChunkData::RESOLUTION * TerrainChunkData::RESOLUTION) + (y * TerrainChunkData::RESOLUTION) + x;
                    data[index] = value;
                }
            }
        }
        return data;
    }
}
