#pragma once
#include <glm/gtx/rotate_vector.hpp>

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

	void draw();

private:
	class Crystal {
	public:
		Crystal(const glm::vec3& pos, const glm::vec3& dir);

		void draw();

	private:
		Mesh mesh;
	};

	std::vector<Crystal*> crystals;
};