#pragma once

#include <cstdint>

#include "FastNoiseLite.hpp"
#include "glm/ext/vector_int3.hpp"

#include "Engine/Systems/Terrain/TerrainTypes.hpp"

class TerrainSystem {
public:
    uint16_t* HeightmapsBuffer_MappedMem;
    ChunkHeightmapLink* ChunkLinksBuffer_MappedMem;

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

    void FeedTerrainRenderer(ChunkHeightmapLink* ChunkLinkMap, uint16_t* HeightmapMap);
private:
    void FullWriteChunkData();

    void WriteChunk(glm::ivec2 ChunkPos, uint16_t* ChunkBegin);
};
