#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

namespace TerrainChunkData {
    struct ChunkData {
        glm::ivec2 worldPos;
        uint32_t instanceId;
        uint32_t isVisible;
    };

    const uint32_t INSTANCE_COUNT = 9; // The number of terrains to be draw per call

    const uint32_t RESOLUTION = 64; // Width & Height

    const uint32_t INDEX_COUNT = (RESOLUTION - 1) * (RESOLUTION - 1) * 6;

    const static inline std::vector<uint32_t> getIndices() {
        std::vector<uint32_t> indices;
        indices.reserve(INDEX_COUNT);

        for (int z = 0; z < RESOLUTION - 1; z++) {
            for (int x = 0; x < RESOLUTION - 1; x++) {
                // Calculate the index of the current vertex and neighbors
                uint32_t topLeft = (z * RESOLUTION) + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = ((z + 1) * RESOLUTION) + x;
                uint32_t bottomRight = bottomLeft + 1;

                // Triangle 1 (Top-Left -> Bottom-Left -> Top-Right)
                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                // Triangle 2 (Top-Right -> Bottom-Left -> Bottom-Right)
                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }
        return indices;
    }

    static std::vector<ChunkData> generateChunkGrid(uint32_t n) {
        std::vector<ChunkData> chunks;
        chunks.reserve(n * n);

        uint32_t currentId = 0;

        for (uint32_t x = 0; x < n; ++x) {
            for (uint32_t y = 0; y < n; ++y) {
                ChunkData chunk;
                chunk.worldPos = {x, y};
                chunk.instanceId = currentId;
                chunk.isVisible = 1;

                chunks.push_back(chunk);
                currentId++;
            }
        }

        return chunks;
    }
};
