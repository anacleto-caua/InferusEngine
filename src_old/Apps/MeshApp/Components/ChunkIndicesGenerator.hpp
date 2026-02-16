#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

#include "TerrainConfig.hpp"

namespace ChunkIndicesGenerator {

    const uint32_t INDEX_COUNT = (TerrainConfig::RESOLUTION - 1) * (TerrainConfig::RESOLUTION - 1) * 6;

    std::vector<uint32_t> getIndices();
};
