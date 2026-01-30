#pragma once

#include <vector>
#include <cstdint>

#include <glm/glm.hpp>

#include "TerrainConfig.hpp"

namespace ChunkIndicesGenerator {

    const uint32_t INDEX_COUNT = (TerrainConfig::INDICES_COUNT - 1) * (TerrainConfig::INDICES_COUNT - 1) * 6;

    std::vector<uint32_t> getIndices();
};
