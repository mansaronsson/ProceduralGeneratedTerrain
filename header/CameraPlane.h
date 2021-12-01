#pragma once
#include <glm/glm.hpp>

struct CameraPlane
{
    glm::vec3 point;
    glm::vec3 normal;

    float evaluatePlane(glm::vec3 p) const {
        return normal.x * (p.x - point.x) + normal.y * (p.y - point.y) + normal.z * (p.z - point.z);
    }
};