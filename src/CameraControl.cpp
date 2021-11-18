#include "..\header\CameraControl.h"

void CameraControl::setMouseStartPos(GLFWwindow* window)
{
    glfwGetCursorPos(window, &mouseX, &mouseY);
}

void CameraControl::moveCamera(const glm::vec3& move)
{
	translation = glm::translate(translation, move);
}

const unsigned int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;

void CameraControl::pollMouse(GLFWwindow* window)
{
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {

        //Get current mouse position
        double currentX = 0.0, currentY = 0.0;
        glfwGetCursorPos(window, &currentX, &currentY);
        //Compute how much mouse moved in 1 frame
        double xoffset = currentX - mouseX;
        double yoffset = currentY - mouseY;
        
        theta += M_PI * xoffset / SCREEN_WIDTH;
        if (theta >= M_PI * 2.0)
            theta = fmod(theta, M_PI * 2.0);
        if (theta < 0.0)
            theta += M_PI * 2.0;

        phi += M_PI * yoffset / SCREEN_HEIGHT;
        if (phi >= M_PI / 2.0)
            phi = M_PI / 2.0; //clamp 90
        if (phi < -M_PI / 2.0)
            phi = -M_PI / 2.0; //calmp -90

        //compute rotation matrix around X first then Y 
        rotation = glm::mat4(1.0f);
        rotation = glm::rotate(rotation, theta, glm::vec3(0.0f, 1.0f, 0.0f));
        rotation = glm::rotate(rotation, phi, glm::vec3(1.0f, 0.0f, 0.0f));

        mouseX = currentX;
        mouseY = currentY;
    }
}

const glm::mat4& CameraControl::computeCameraMatrix()
{
    result = glm::mat4{ 1.0f };
    //sjukt oklart med ordningen här, känns som att man får bättre controls om man gör rotation först och sen translation
    result = result * rotation;
    result = glm::translate(result, startposition);
    result = result * translation;

    return result;
}

void CameraControl::resetCamera()
{
	rotation = glm::mat4{ 1.0f };
	phi = 0.0f;
	theta = 0.0f;
}
