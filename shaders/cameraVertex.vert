#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 InvP, P, V;
void main() {

    //    glm::vec4 v = invproj * glm::vec4(cameraPoints[i], cameraPoints[i + 1], cameraPoints[i + 2], 1.0f);
    //    glm::vec3 worldV = v / v.w;
    vec4 v = InvP * vec4(position, 1.0f);
    vec3 worldPos = vec3(v.x / v.w, v.y / v.w, v.z / v.w);

	gl_Position =   P *  V *  vec4(worldPos, 1.0f);
}