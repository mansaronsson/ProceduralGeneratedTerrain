#pragma once
#include <glm/gtc/noise.hpp>
#include <glm/gtx/transform.hpp>

#include <vector>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Mesh.h"


struct CrystalInfo {
	// Gather all data about a chunk here
};

class CrystalChunk {
public:
	CrystalChunk(const glm::vec3& pos, const glm::vec3& dir);

	~CrystalChunk();

	void bake();
	void draw();

private:
	class Crystal {
	public:
		Crystal(const glm::vec3& pos, const glm::vec3& dir);

		~Crystal();

		void bake();
		void draw();

	private:
		std::vector<unsigned int> m_indices;
		std::vector<Vertex> m_vertices;

		Mesh mesh;
	};

	std::vector<Crystal*> crystals;
};