#pragma once

#include <array>
#include <glm/fwd.hpp>
#include <vector>
#include <cstdint>

#include <FastNoiseLite.hpp>

#include "TerrainConfig.hpp"

namespace NoiseGenerator {
    inline std::vector<uint16_t> generateMockHeightmaps() {
        // Total pixels = Width * Height * Layers
        std::vector<uint16_t> data(TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION * TerrainConfig::INSTANCE_COUNT);

        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);

        for (int layer = 0; layer < TerrainConfig::INSTANCE_COUNT; layer++) {
            // Calculate which grid cell this layer belongs to (0..2, 0..2)
            int gridX = layer % 3;
            int gridY = layer / 3;

            for (int y = 0; y < TerrainConfig::RESOLUTION; y++) {
                for (int x = 0; x < TerrainConfig::RESOLUTION; x++) {
                    // Global coordinates for noise
                    float globalX = (gridX * (TerrainConfig::RESOLUTION - 1)) + x;
                    float globalY = (gridY * (TerrainConfig::RESOLUTION - 1)) + y;

                    float height = noise.GetNoise(globalX, globalY);
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

    inline std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION> genChunk(glm::ivec2 worldPos) {
        FastNoiseLite noise;
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION> data;

        uint32_t index = 0;
        float globalX, globalZ;
        for (int x = 0; x < TerrainConfig::RESOLUTION; x++) {
            globalX = x + (TerrainConfig::RESOLUTION * worldPos.x);
            for (int z = 0; z < TerrainConfig::RESOLUTION; z++) {
                globalZ = z + (TerrainConfig::RESOLUTION * worldPos.y);

                data[index] = noise.GetNoise(globalX, globalZ);
                index++;
            }
        }

        return data;
    }
}
