#pragma once

#include <array>
#include <cstdint>

#include <glm/fwd.hpp>
#include <FastNoiseLite.hpp>

#include "TerrainConfig.hpp"

namespace NoiseGenerator {
    inline std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION> genChunk(FastNoiseLite noise, glm::ivec2 worldPos) {
        std::array<uint16_t, TerrainConfig::RESOLUTION * TerrainConfig::RESOLUTION> data;

        int32_t TerrainRes = TerrainConfig::RESOLUTION;

        uint32_t index = 0;
        float globalX, globalZ;
        for (int32_t x = 0; x < TerrainRes; x++) {
            globalX = x + ((TerrainRes-1) * worldPos.x);
            for (int32_t z = 0; z < TerrainRes; z++) {
                globalZ = z + ((TerrainRes-1) * worldPos.y);

                float n = noise.GetNoise(globalX, globalZ);
                float remapped = (n + 1.0f) * 0.5f * 65535.0f;

                data[index] = static_cast<uint16_t>(remapped);
                index++;
            }
        }

        return data;
    }
}
