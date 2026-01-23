#version 450

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 debugColor;

layout(push_constant) uniform PushConstants {
    mat4 lookAt;
    vec4 data;
} terrain_push;

// The sqrt() of the number of world chunks to be drawn per call
// Should match with sqrt(TerrainChunkData::INSTANCE_COUNT)
const int GRID_WIDTH = 3;

const int RESOLUTION = 64;
const float GRID_SIZE = 20.0; // Chunk size in world units

void main() {
    // Current pos
    int instanceX = gl_InstanceIndex % GRID_WIDTH;
    int instanceZ = gl_InstanceIndex / GRID_WIDTH;

    // Global pos (x, z)
    float offsetX = float(instanceX) * GRID_SIZE;
    float offsetZ = float(instanceZ) * GRID_SIZE;

    // UVs (0..1)
    int xIndex = gl_VertexIndex % RESOLUTION;
    int zIndex = gl_VertexIndex / RESOLUTION;

    float u = float(xIndex) / float(RESOLUTION - 1);
    float v = float(zIndex) / float(RESOLUTION - 1);
    texCoord = vec2(u, v);

    // Local Position(center on (0, 0, 0))
    float x = (u - .5) * GRID_SIZE;
    float z = (v - .5) * GRID_SIZE;

    // vec3 localPos = vec3(x, 0.0, z);
    vec3 worldPos = vec3(x + offsetX, 0.0, z + offsetZ);

    // 3. Standard Camera Transform
    gl_Position = terrain_push.lookAt * vec4(worldPos, 1.0);
    // Debug: Color checkerboard based on instance ID
    bool checker = ((instanceX + instanceZ) % 2) == 0;
    debugColor = checker ? vec3(0.8, 0.2, 0.2) : vec3(0.2, 0.2, 0.8);
}