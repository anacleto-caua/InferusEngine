#include "Camera3D.hpp"

#include <glm/ext/matrix_transform.hpp>

#include "Default.hpp"

void Camera3D::Init(float CurrAspect, glm::mat4* pModelViewProjection) {
    const constexpr glm::vec3 STARTING_LOOKING_AT = glm::vec3(0.0f, 0.0f, 0.0f);
    const constexpr glm::vec3 STARTING_CAM_POS    = glm::vec3(-15.0f, 25.0f, -15.0f);
    const constexpr glm::float32_t STARTING_POV   = 90.0;

    ModelViewProjection = pModelViewProjection;

    FOV = STARTING_POV;
    Position = STARTING_CAM_POS;
    FocalLength = 1.0f / tan(glm::radians(FOV) / 2.0f);

    Projection = glm::mat4(0.0f);
    Projection[1][1] = -FocalLength;
    Projection[2][2] = 0.0f;
    Projection[2][3] = -1.0f;
    Projection[3][2] = 0.1f;

    View = glm::lookAt(Position, STARTING_LOOKING_AT, Vector3::UP);
    Model = glm::mat4(1.0f);

    Resize(CurrAspect);
}

Camera3D::~Camera3D(){
    // ...
}

void Camera3D::Resize(float NewAspect) {
    Aspect = NewAspect;
    Projection[0][0] = FocalLength / NewAspect;
    RefreshMVP();
}

void Camera3D::RefreshMVP() {
    *ModelViewProjection = Projection * View * Model;
}
