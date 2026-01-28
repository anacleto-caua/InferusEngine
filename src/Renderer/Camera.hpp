#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

class Camera {
public:
    glm::mat4* pMvp;
private:
    glm::float32_t pov;
    glm::float32_t focalLength;
    glm::float32_t aspect;
    glm::mat4 projection;
    glm::vec3 camPos;
    glm::vec3 target;
    glm::mat4 view;
    glm::mat4 model;
public:
    Camera() = default;
    ~Camera() = default;
    Camera(const Camera&) = delete;
    Camera& operator=(const Camera&) = delete;

    void init(glm::mat4* pMvp);
    void setAspect(float aspect);
    void refreshMvp();
private:
};
