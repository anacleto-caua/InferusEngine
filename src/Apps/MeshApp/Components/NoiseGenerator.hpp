#pragma once

#include <vector>
#include <cstdint>

#include "TerrainConfig.hpp"

namespace NoiseGenerator {
    inline std::vector<uint16_t> generateMockHeightmaps() {
        // Total pixels = Width * Height * Layers
        std::vector<uint16_t> data(TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION * TerrainConfig::INSTANCE_COUNT);

        for (int layer = 0; layer < TerrainConfig::INSTANCE_COUNT; layer++) {
            // Calculate which grid cell this layer belongs to (0..2, 0..2)
            int gridX = layer % 3;
            int gridY = layer / 3;

            for (int y = 0; y < TerrainConfig::RESOLUTION; y++) {
                for (int x = 0; x < TerrainConfig::RESOLUTION; x++) {
                    // Global coordinates for noise
                    float globalX = (gridX * (TerrainConfig::RESOLUTION - 1)) + x;
                    float globalY = (gridY * (TerrainConfig::RESOLUTION - 1)) + y;

                    // Simple sine wave terrain
                    float height = std::sin(globalX * 0.1f) + std::cos(globalY * 0.1f);

                    // Normalize to 0..1 range roughly
                    height = (height * 0.25f) + 0.5f;

                    // Convert to 16-bit integer (0 to 65535)
                    uint16_t value = static_cast<uint16_t>(height * 65535.0f);

                    // Index the texture array
                    // [Layer][Row][Col]
                    int index = (layer * TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION) + (y * TerrainConfig::RESOLUTION) + x;
                    data[index] = value;
                }
            }
        }
        return data;
    }
}
