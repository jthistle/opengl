#include "camera.h"

Camera::Camera(float fov, float aspect, float near, float far)
{
    position   = glm::vec3(0.0f, 0.0f,  3.0f);
    cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

    projection = glm::perspective(glm::radians(fov), aspect, near, far);
}

glm::mat4 Camera::generateView()
{
    // Generate direction vector
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);

    return glm::lookAt(position, position + cameraFront, cameraUp);
}

void Camera::move(Camera::MoveDirection direction, float distance)
{
    switch (direction) {
        case Camera::MoveDirection::FORWARD:
            position += distance * glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
            break;
        case Camera::MoveDirection::BACKWARD:
            position -= distance * glm::normalize(glm::vec3(cameraFront.x, 0.0f, cameraFront.z));
            break;
        case Camera::MoveDirection::LEFT:
            position -= glm::normalize(glm::cross(cameraFront, cameraUp)) * distance;
            break;
        case Camera::MoveDirection::RIGHT:
            position += glm::normalize(glm::cross(cameraFront, cameraUp)) * distance;
            break;
        case Camera::MoveDirection::UP:
            position += distance * cameraUp;
            break;
        case Camera::MoveDirection::DOWN:
            position -= distance * cameraUp;
            break;
    }
}

void Camera::mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastMouse = glm::vec2(xpos, ypos);
        firstMouse = false;
        return;
    }

    float xoffset = xpos - lastMouse.x;
    float yoffset = lastMouse.y - ypos; // reversed since y-coordinates range from bottom to top
    lastMouse = glm::vec2(xpos, ypos);

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Clamp pitch
    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

}

void Camera::configureShader(Shader &shader) {
    shader.use();
    shader.setMat4("view", generateView());
    shader.setMat4("projection", projection); 
    shader.setVec3("viewPos", position);
}