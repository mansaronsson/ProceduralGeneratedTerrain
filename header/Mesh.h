#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <string>
#include <vector>


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal = glm::vec3{ 0.0f };
	glm::vec3 color = glm::vec3{ 0.0f };
	glm::vec3 heatcolor = glm::vec3{ 0.0f };
	glm::vec3 moistcolor = glm::vec3{ 0.0f };
	glm::vec3 biomecolor = glm::vec3{ 0.0f };
	glm::vec3 normalcolor = glm::vec3{ 0.0f };
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	Mesh() = default;
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

	/// <summary>
	/// Remove buffer objects from VRAM
	/// </summary>
	void deleteMesh();
	
	void draw(int polygonMode);
private:
	unsigned int VAO, VBO, EBO;
	bool bakedMesh = false;

	void setupMesh();
};