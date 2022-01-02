#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <string>
#include <vector>


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal{ 0.0f, 1.0f, 0.0f };
	glm::vec3 lodColor{ 0.0f };
	float biome{ 0 };
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