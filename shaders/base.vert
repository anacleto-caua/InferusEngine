#version 450

layout( push_constant ) uniform constants
{
    vec4 data;
    mat4 renderMatrix;
} PushConstants;

layout(location = 0) out vec3 fragColor;


void main()
{
    vec3 positions[3] = vec3[](
        vec3(0.5, 0.5, 0.0),
        vec3(0.0, -0.5, 0.0),
        vec3(-0.5, 0.5, 0.0)
    );

    vec3 colors[3] = vec3[](
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    gl_Position = PushConstants.renderMatrix * vec4(positions[gl_VertexIndex], 1.0);

    fragColor = colors[gl_VertexIndex];
}