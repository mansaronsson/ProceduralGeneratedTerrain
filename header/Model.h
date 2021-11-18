#pragma once

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <string>
#include <vector>


struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices);
	
	void draw(int polygonMode);
private:
	unsigned int VAO, VBO, EBO;

	void setupMesh();
};