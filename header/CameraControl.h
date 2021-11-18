#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define _USE_MATH_DEFINES
#include <math.h>

class CameraControl {
public:
	CameraControl(const glm::vec3& startpos) : startposition{ startpos }, phi{ 0.0f }, theta{ 0.0f },
		mouseX{ 0.0 }, mouseY{ 0.0 }, rotation{ 1.0f }, translation{ 1.0f }, result{ 1.0f } {}

	void setMouseStartPos(GLFWwindow* window);

	/// <summary>
	/// Translates the camera x,y,z amount from current position
	/// </summary>
	/// <param name="move"></param>
	void moveCamera(const glm::vec3& move);

	/// <summary>
	/// 
	/// </summary>
	/// <param name="window"></param>
	void pollMouse(GLFWwindow* window);

	/// <summary>
	/// Compute the view matrix rototation * translation of the camera
	/// </summary>
	/// <returns></returns>
	const glm::mat4& computeCameraMatrix();

	/// <summary>
	/// Reset rotation of camera, position is unchanged
	/// </summary>
	void resetCamera();

private:
	float phi, theta;
	double mouseX, mouseY;
	glm::mat4 rotation, translation;
	glm::vec3 startposition;
	glm::mat4 result;
};