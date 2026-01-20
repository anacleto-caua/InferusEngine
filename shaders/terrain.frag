#version 450

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

void main()
{
    // Output a generic UV gradient
    // Bottom-Left = Black
    // Top-Right = Yellow
    outColor = vec4(texCoord.x, texCoord.y, 0.0, 1.0);
}