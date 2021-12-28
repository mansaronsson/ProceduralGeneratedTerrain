#pragma once
#include <glm/gtx/rotate_vector.hpp>

#include <vector>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "Mesh.h"

class Crystal {
public:
	Crystal(glm::vec3 pos, glm::vec3 dir);

	void draw();

private:
	Mesh mesh;
};