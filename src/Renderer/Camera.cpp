#include "Camera.hpp"

#include <stdexcept>

#include <glm/fwd.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Default.hpp"

void Camera::init(glm::mat4* pMvp) {
    const constexpr glm::vec3 STARTING_LOOKING_AT = glm::vec3(0.0f, 0.0f, 0.0f);
    const constexpr glm::vec3 STARTING_CAM_POS    = glm::vec3(-15.0f, 25.0f, -15.0f);
    const constexpr glm::float32_t STARTING_POV   = 90.0;

    this->pMvp = pMvp;

    pov     = STARTING_POV;
    camPos  = STARTING_CAM_POS;
    target  = STARTING_LOOKING_AT;
    focalLength = 1.0f / tan(glm::radians(pov) / 2.0f);

    projection = glm::mat4(0.0f);
    projection[1][1] = -focalLength;
    projection[2][2] = 0.0f;
    projection[2][3] = -1.0f;
    projection[3][2] = 0.1f;

    view = glm::lookAt(camPos, target, vector::up);
    model = glm::mat4(1.0f);
}

void Camera::setAspect(float aspect) {
    this->aspect = aspect;
    projection[0][0] = focalLength / aspect;
    refreshMvp();
}

void Camera::refreshMvp() {
    if (pMvp == nullptr) {
        throw std::runtime_error("pointer to model view projection matrix is nullptr");
    }
    *pMvp = projection * view * model;
}
