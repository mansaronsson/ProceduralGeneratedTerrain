#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <math.h>

enum CameraMovement {
	FORWARD, BACKWARD, LEFT, RIGHT, ASCEND, DESCEND
};

class CameraControl {
public:
	CameraControl(const glm::vec3& startpos) : position{ startpos }, up{ glm::vec3{0.0f, 1.0f, 0.0f} }, front{ glm::vec3{0.0f, 0.0f, -1.0f} }, Yaw{ 0.0f },
		Pitch{ 0.0f }, mouseX{ 0.0 }, mouseY{ 0.0 }, cameraSpeed{ 5.0f } {
		worldUp = up;
		updateCameraVectors();
	}

	/// <summary>
	/// Get the start location of mouse when first start draging
	/// </summary>
	/// <param name="window"></param>
	void setMouseStartPos(GLFWwindow* window);

	/// <summary>
	/// Translates the camera x,y,z amount from current position
	/// </summary>
	/// <param name="move"></param>
	void moveCamera(CameraMovement direction, float deltaTime);

	/// <summary>
	/// Poll mouse movement and update rotation accordingly 
	/// </summary>
	/// <param name="window"></param>
	void pollMouse(GLFWwindow* window, unsigned int SCREEN_WIDTH, unsigned int SCREEN_HEIGHT);

	/// <summary>
	/// Compute the view matrix for camera 
	/// </summary>
	/// <returns></returns>
	const glm::mat4& computeCameraViewMatrix();

	/// <summary>
	/// Reset rotation of camera, position is unchanged
	/// </summary>
	void resetCamera();

	glm::vec3 getCameraPosition() const;

private:
	/// <summary>
	/// Recompute front, up and right vectors
	/// </summary>
	void updateCameraVectors();

	float Yaw, Pitch;
	double mouseX, mouseY;
	float cameraSpeed;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;	
};