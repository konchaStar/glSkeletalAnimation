#include "Camera.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Camera::Camera(float fovDeg, float aspect, float zNear, float zFar, glm::vec3 position) {
    this->projection = glm::perspective(glm::radians(fovDeg), aspect, zNear, zFar);
    this->position = position;
    this->model = glm::lookAt(position, position - glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
}

void Camera::translate(glm::vec3 translation) {
    this->position += translation;
}

void Camera::rotate(float angleDeg, glm::vec3 axis) {
    this->model = glm::rotate(this->model, glm::radians(angleDeg), axis);
}

glm::mat4 Camera::getViewMatrix() {
    return glm::translate(model, -position);
}

glm::vec3 Camera::lookAt() {
    return glm::vec3(this->model[0].z, this->model[1].z, this->model[2].z);
}

glm::vec3 Camera::up() {
    return glm::vec3(this->model[0].y, this->model[1].y, this->model[2].y);
}

glm::vec3 Camera::right() {
    return glm::vec3(this->model[0].x, this->model[1].x, this->model[2].x);
}

void Camera::recalculateProjectionMatrix(float fovDeg, float aspect, float zNear, float zFar) {
    projection = glm::perspective(glm::radians(fovDeg), aspect, zNear, zFar);
}

