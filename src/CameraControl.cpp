#include "..\header\CameraControl.h"

void CameraControl::updateCameraVectors()
{
    glm::vec3 _front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front = glm::normalize(front);

    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void CameraControl::setMouseStartPos(GLFWwindow* window)
{
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

void CameraControl::moveCamera(CameraMovement direction, float deltaTime)
{  
    float velocity = cameraSpeed * deltaTime;
    if (direction == FORWARD)
        position += front * velocity;
    if(direction == BACKWARD)
        position -= front * velocity;
    if (direction == LEFT)
        position -= right * velocity;
    if (direction == RIGHT)
        position += right * velocity;
    if (direction == ASCEND)
        position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
    if(direction == DESCEND)
        position -= glm::vec3(0.0f, 1.0f, 0.0f) * velocity;

}

//const unsigned int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

void CameraControl::pollMouse(GLFWwindow* window, unsigned int SCREEN_WIDTH, unsigned int SCREEN_HEIGHT)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

        //Get current mouse position
        double currentX = 0.0, currentY = 0.0;
        glfwGetCursorPos(window, &currentX, &currentY);
        //Compute how much mouse moved in 1 frame
        double xoffset = currentX - mouseX;
        double yoffset = mouseY - currentY; 
        
        //Yaw += M_PI * xoffset / SCREEN_WIDTH;
        Yaw += 90.0f * xoffset / SCREEN_WIDTH;
        //if (Yaw >= M_PI * 2.0)
        //    Yaw = fmod(Yaw, M_PI * 2.0);
        //if (Yaw < 0.0)
        //    Yaw += M_PI * 2.0;

        //Pitch += M_PI * yoffset / SCREEN_HEIGHT;
        Pitch += 90.0f * yoffset / SCREEN_HEIGHT;
        if (Pitch >= 89.0f)
            Pitch = 89.0f; //clamp 90
        if (Pitch < -89.0f)
            Pitch = -89.0f; //calmp -90

        //compute rotation matrix around X first then Y 
        //rotation = glm::mat4(1.0f);
        //rotation = glm::rotate(rotation, phi, glm::vec3(1.0f, 0.0f, 0.0f));
        //rotation = glm::rotate(rotation, theta, glm::vec3(0.0f, 1.0f, 0.0f));

        updateCameraVectors();

        mouseX = currentX;
        mouseY = currentY;
    }
}

const glm::mat4& CameraControl::computeCameraViewMatrix()
{
    //result = glm::mat4{ 1.0f };
    ////sjukt oklart med ordningen här, känns som att man får bättre controls om man gör rotation först och sen translation
    //result = result * rotation;
    //result = glm::translate(result, startposition);
    //result = result * translation;

    //return result;
    return glm::lookAt(position, position + front, up);
}

void CameraControl::resetCamera()
{
	//rotation = glm::mat4{ 1.0f };
	//phi = 0.0f;
	//theta = 0.0f;
    Yaw = 0.0f;
    Pitch = 0.0f;
    updateCameraVectors();
}

glm::vec3 CameraControl::getCameraPosition() const
{
    //glm::vec3 position = startposition;
    //return glm::vec3(translation * glm::vec4(position, 1.0f));
    return position;
}
