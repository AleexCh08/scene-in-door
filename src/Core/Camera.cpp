#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), RotationSpeed(50.0f) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(int direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    // float rotation = RotationSpeed * deltaTime;

    if (direction == 0) Position += Front * velocity; // Adelante
    if (direction == 1) Position -= Front * velocity; // Atras 
    if (direction == 2) Position += Left * velocity; // Izquierda
    if (direction == 3) Position += Right * velocity; // Derecha
    if (direction == 4) Position += Up * velocity; // Arriba
    if (direction == 5) Position -= Up * velocity; // Abajo

    /*if (direction == 2) Yaw -= rotation; // Rotar izquierda
    if (direction == 3) Yaw += rotation; // Rotar derecha
    if (direction == 4) Pitch += rotation; // Rotación arriba
    if (direction == 5) Pitch -= rotation; // Rotación abajo*/
    
    if (Pitch > 89.0f) Pitch = 89.0f;
    if (Pitch < -89.0f) Pitch = -89.0f;
    updateCameraVectors();
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Left = -Right;
    Up = glm::normalize(glm::cross(Right, Front));
}