#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>

class Camera {

public:
    glm::mat4 model;
    glm::mat4 projection;
    glm::vec3 position;
    Camera(float fovDeg, float aspect, float zNear, float zFar, glm::vec3 position);
    void recalculateProjectionMatrix(float fovDeg, float aspect, float zNear, float zFar);
    void translate(glm::vec3 translation);
    void rotate(float angleDeg, glm::vec3 axis);
    void rotateRelatively(float angleDeg);
    glm::mat4 getViewMatrix();
    glm::vec3 lookAt();
    glm::vec3 up();
    glm::vec3 right();
};

#endif
