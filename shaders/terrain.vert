#version 450

// 1. Define the struct exactly as it is in C++
// Ensure your C++ struct aligns to 16 bytes (ivec2 + uint + uint = 16 bytes)
struct ChunkData {
    ivec2 worldPos;  // The grid coordinate (e.g., 1, 0)
    uint instanceId;
    uint isVisible;
};

// 2. Bind the Buffer
// This allows us to access the specific data for THIS instance
layout(std430, set = 0, binding = 1) readonly buffer ChunkBuffer {
    ChunkData chunks[]; 
} chunkDataBuffer;

layout(location = 0) out vec2 texCoord;
layout(location = 1) out vec3 debugColor;

layout(push_constant) uniform PushConstants {
    mat4 lookAt;
    vec4 data;
} terrain_push;

const int RESOLUTION = 64;
const float GRID_SIZE = 20.0; // Size of one chunk in world units

void main() {
    // 3. Retrieve the data for the current chunk instance
    ChunkData currentChunk = chunkDataBuffer.chunks[gl_InstanceIndex];

    // Optional: Quick GPU-side culling
    // If the CPU says it's not visible, collapse the vertex so it doesn't render
    if (currentChunk.isVisible == 0) {
        gl_Position = vec4(0.0/0.0); // NaNs usually discard primitive, or just scale to 0
        return;
    }

    // 4. Calculate the World Offset
    // We multiply the integer grid coordinate by the size of the chunk
    float chunkOffsetX = float(currentChunk.worldPos.x) * GRID_SIZE;
    float chunkOffsetZ = float(currentChunk.worldPos.y) * GRID_SIZE;

    // --- Standard Plane Generation (Local Space) ---
    // This generates a flat mesh centered at (0,0) locally
    int xIndex = gl_VertexIndex % RESOLUTION;
    int zIndex = gl_VertexIndex / RESOLUTION;

    float u = float(xIndex) / float(RESOLUTION - 1);
    float v = float(zIndex) / float(RESOLUTION - 1);
    texCoord = vec2(u, v);

    // Local X/Z range: from -10.0 to +10.0 (if GRID_SIZE is 20)
    float localX = (u - 0.5) * GRID_SIZE;
    float localZ = (v - 0.5) * GRID_SIZE;

    // 5. Apply the Translation
    // Final Pos = Local Mesh Position + Chunk World Offset
    vec3 finalWorldPos = vec3(localX + chunkOffsetX, 0.0, localZ + chunkOffsetZ);

    // 6. Project to Screen
    gl_Position = terrain_push.lookAt * vec4(finalWorldPos, 1.0);

    // Debug: Checkerboard based on actual world coordinates
    bool checker = ((currentChunk.worldPos.x + currentChunk.worldPos.y) % 2) == 0;
    debugColor = checker ? vec3(0.8, 0.2, 0.2) : vec3(0.2, 0.2, 0.8);
}