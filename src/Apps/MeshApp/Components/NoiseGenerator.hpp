#pragma once

#include <array>
#include <glm/fwd.hpp>
#include <vector>
#include <cstdint>

#include <FastNoiseLite.hpp>

#include "TerrainConfig.hpp"

namespace NoiseGenerator {
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

                float n = noise.GetNoise(globalX, globalZ);
                float remapped = (n + 1.0f) * 0.5f * 65535.0f;

                data[index] = static_cast<uint16_t>(remapped);
                index++;
            }
        }

        return data;
    }
}
