#pragma once
#include <glm/glm.hpp>
#include "Mesh.h"
#include <vector>


class BoundingBox {
public:
	BoundingBox() = default;
	BoundingBox(const std::vector<glm::vec3>& points) {
		//p1-p4 are top vertices of bounding box given clockwise
		//p5-p8 are bottom verticies given clockwise
		std::vector<Vertex> vertices;
		for (auto p : points) {
			vertices.push_back(Vertex{ p });
		}
		std::vector<unsigned int> indices{
			0, 3, 2, 0, 2, 1, //top
			4, 6, 7, 4, 5, 6, //bottom
			3, 4, 7, 3, 0, 4, //left
			2, 6, 5, 2, 5, 1, //right
			3, 7, 6, 3, 6, 2, //front
			0, 5, 4, 0, 1, 5 //back

		};
		boundingMesh = Mesh{vertices, indices};
	}

	void draw() {
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		boundingMesh.draw(GL_TRIANGLES);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);
	}

	// glm::vec3 p1, p2, p3, p4, p5, p6, p7, p8;
	glm::vec3 getPoint(unsigned int nr) const {
		return boundingMesh.vertices[nr].position;
	}

private:
	Mesh boundingMesh;

};