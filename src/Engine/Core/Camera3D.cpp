#include "Camera3D.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "Default.hpp"
#include "Engine/Core/InputSystem.hpp"

// This class is most definetvelly overthinked with all this pointer stuff,
// I could most certainlly simply recalculate the matrix per frame.

void Camera3D::Init(float CurrAspect, glm::mat4* pModelViewProjection) {
    const constexpr glm::float32_t STARTING_POV   = 90.0;

    Position = { -10, 10, -10 };
    LookAt = { 0, 0, 0 };

    ModelViewProjection = pModelViewProjection;

    FOV = STARTING_POV;
    FocalLength = 1.0f / tan(glm::radians(FOV) / 2.0f);

    Projection = glm::mat4(0.0f);
    Projection[1][1] = -FocalLength;
    Projection[2][2] = 0.0f;
    Projection[2][3] = -1.0f;
    Projection[3][2] = 0.1f;

    Model = glm::mat4(1.0f);

    Move();
    Resize(CurrAspect);

    FrameMovement = Vector3::ZERO;

    InputSystem::RegisterCallback(InputSystem::InfKey::Forward, [this](void){ this->FrameMovement+=Vector3::FORWARD; });
    InputSystem::RegisterCallback(InputSystem::InfKey::Backward, [this](void){ this->FrameMovement-=Vector3::FORWARD; });

    InputSystem::RegisterCallback(InputSystem::InfKey::Right, [this](void){ this->FrameMovement+=Vector3::RIGHT; });
    InputSystem::RegisterCallback(InputSystem::InfKey::Left, [this](void){ this->FrameMovement-=Vector3::RIGHT; });

    InputSystem::RegisterCallback(InputSystem::InfKey::Up, [this](void){ this->FrameMovement+=Vector3::UP; });
    InputSystem::RegisterCallback(InputSystem::InfKey::Down, [this](void){ this->FrameMovement-=Vector3::UP; });
}

Camera3D::~Camera3D(){
    // ...
}

void Camera3D::Resize(float NewAspect) {
    Aspect = NewAspect;
    Projection[0][0] = FocalLength / NewAspect;
    RefreshMVP();
}

void Camera3D::Move() {
    View = glm::lookAt(Position, LookAt, Vector3::UP);
    RefreshMVP();
}

void Camera3D::RefreshMVP() {
    *ModelViewProjection = Projection * View * Model;
}

void Camera3D::Update(float DeltaTime) {
    if (FrameMovement != Vector3::ZERO) {
        FrameMovement = glm::normalize(FrameMovement);
        Position += FrameMovement * SPEED * DeltaTime;
        Move();
        FrameMovement = Vector3::ZERO;
    }
}
