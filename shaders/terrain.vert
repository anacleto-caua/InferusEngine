#version 450

layout(location = 0) out vec2 texCoord;

layout(push_constant) uniform PushConstants {
    mat4 lookAt;
    vec4 data;
} terrain_push;

const int RESOLUTION = 64;
const float GRID_SIZE = 20.0; // Chunk size in world units

void main() {
    // UVs (0..1)
    int xIndex = gl_VertexIndex % RESOLUTION;
    int zIndex = gl_VertexIndex / RESOLUTION;

    float u = float(xIndex) / float(RESOLUTION - 1);
    float v = float(zIndex) / float(RESOLUTION - 1);
    texCoord = vec2(u, v);

    // Local Position
    // We subtract 0.5 so the grid is centered at (0,0,0)
    // Range: -10.0 to +10.0 on X and Z
    float x = (u - 0.5) * GRID_SIZE;
    float z = (v - 0.5) * GRID_SIZE;

    // Flat Y=0 for now
    vec3 localPos = vec3(x, 0.0, z);

    // 3. Standard Camera Transform
    gl_Position = terrain_push.lookAt * vec4(localPos, 1.0);
}