#version 450

// Terrain resolution - Matches TerrainChunkData.hpp
const int RESOLUTION = 64;
// Chunk size in World Units
const float GRID_SIZE = 5.0;

layout(location = 0) out vec2 texCoord;

void main() {
    // 1. Calculate Integer Grid Coords
    int xIndex = gl_VertexIndex % RESOLUTION;
    int zIndex = gl_VertexIndex / RESOLUTION;

    // 2. Normalize UVs (0..1)
    float u = float(xIndex) / float(RESOLUTION - 1);
    float v = float(zIndex) / float(RESOLUTION - 1);
    texCoord = vec2(u, v);

    // 3. Generate Local Position (Centered at 0,0)
    // x range: -2.0 to 2.0
    // z range: -2.0 to 2.0
    float x = (u - 0.5) * GRID_SIZE;
    float z = (v - 0.5) * GRID_SIZE;

    // 4. "Fake Camera" Transform
    // We manually tilt the mesh so it looks like 3D floor.
    // Rotate 45 degrees around X axis:
    // y' = y*cos - z*sin
    // z' = y*sin + z*cos
    float cosA = 0.707; // cos(45)
    float sinA = 0.707; // sin(45)

    vec3 pos;
    pos.x = x;
    pos.y = -z * sinA;      // Tilt "floor" down
    pos.z = z * cosA + 2.0; // Move it forward 2 units (away from camera)

    // 5. Projection (Perspective Divide)
    // Simple perspective: x / z, y / z
    // We adjust Z slightly so we don't divide by zero
    gl_Position = vec4(pos.x, pos.y, pos.z, pos.z);
    // Note: Putting 'pos.z' in W triggers the hardware perspective divide
}