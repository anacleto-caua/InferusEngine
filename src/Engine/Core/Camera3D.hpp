#pragma once

#include "glm/fwd.hpp"
#include "glm/ext/matrix_float4x4.hpp"

class Camera3D {
public:
private:
    glm::vec3 Position;
    float FOV;
    float Aspect;
    float FocalLength;

    glm::mat4 Model;
    glm::mat4 View;
    glm::mat4 Projection;
    glm::mat4* ModelViewProjection;
public:
    Camera3D() = default;
    ~Camera3D();
    Camera3D(const Camera3D&) = delete;
    Camera3D& operator=(const Camera3D&) = delete;

    void Init(float CurrAspect, glm::mat4* pModelViewProjection);

    void Resize(float NewAspect);

    void RefreshMVP();

private:
};
