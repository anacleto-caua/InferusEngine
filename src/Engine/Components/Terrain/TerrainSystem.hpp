#pragma once

#include <array>
#include <cstdint>

#include "FastNoiseLite.hpp"
#include "glm/ext/vector_int3.hpp"

#include "Engine/Components/Terrain/TerrainTypes.hpp"
#include "Engine/Components/Terrain/TerrainConfig.hpp"

class TerrainSystem {
public:
    // TODO: Consider writing this directly to the mapped hightmap staging buffer
    std::array<uint16_t, TerrainConfig::Heightmap::HEIGHTMAP_ALL_IMAGES_PIXEL_COUNT> HeightmapsBuffer;

    std::array<ChunkHeightmapLink, TerrainConfig::ChunkToHeightmapLinking::INSTANCE_COUNT> ChunkLinksBuffer;
private:
    glm::ivec3* PlayerPos;

    FastNoiseLite BaseNoise;
public:
    TerrainSystem() = default;
    ~TerrainSystem();
    TerrainSystem(const TerrainSystem&) = delete;
    TerrainSystem& operator=(const TerrainSystem&) = delete;

    void Init(glm::ivec3* PlayerPos);
    void Update();
private:
    void FullWriteChunkData();

    void WriteChunk(glm::ivec2 ChunkPos, uint16_t* ChunkBegin);
};
