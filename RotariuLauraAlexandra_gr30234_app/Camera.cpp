#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = -glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        glm::vec3 moveDirection = glm::normalize(cameraTarget - cameraPosition);
        glm::vec3 right = glm::normalize(glm::cross(cameraUpDirection, moveDirection));
        glm::vec3 up = glm::normalize(glm::cross(moveDirection, right));

        if (direction == MOVE_FORWARD) {
            cameraPosition += moveDirection * speed;
            cameraTarget += moveDirection * speed;
        }
        else if (direction == MOVE_BACKWARD) {
            cameraPosition -= moveDirection * speed;
            cameraTarget -= moveDirection * speed;
        }
        else if (direction == MOVE_RIGHT) {
            cameraPosition += right * speed;
            cameraTarget += right * speed;
        }
        else if (direction == MOVE_LEFT) {
            cameraPosition -= right * speed;
            cameraTarget -= right * speed;
        }
        if (direction == MOVE_UP) {
            cameraPosition += up * speed;
            cameraTarget += up * speed;
        }
        else if (direction == MOVE_DOWN) {
            cameraPosition -= up * speed;
            cameraTarget -= up * speed;
        }
    }

    //update the camera internal parameters following a camera rotate event
    void Camera::rotate(float pitch, float yaw) {
        const float sensitivity = 0.1f;
        pitch *= sensitivity;
        yaw *= sensitivity;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFrontDirection = glm::normalize(front);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));
        cameraTarget = cameraPosition + cameraFrontDirection;
    }
}
