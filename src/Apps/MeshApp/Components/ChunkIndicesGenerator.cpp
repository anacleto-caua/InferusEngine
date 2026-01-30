#include "ChunkIndicesGenerator.hpp"

namespace ChunkIndicesGenerator {
        std::vector<uint32_t> getIndices() {
        std::vector<uint32_t> indices;
        indices.reserve(INDEX_COUNT);

        for (int z = 0; z < TerrainConfig::RESOLUTION - 1; z++) {
            for (int x = 0; x < TerrainConfig::RESOLUTION - 1; x++) {
                // Calculate the index of the current vertex and neighbors
                uint32_t topLeft = (z * TerrainConfig::RESOLUTION) + x;
                uint32_t topRight = topLeft + 1;
                uint32_t bottomLeft = ((z + 1) * TerrainConfig::RESOLUTION) + x;
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
};
