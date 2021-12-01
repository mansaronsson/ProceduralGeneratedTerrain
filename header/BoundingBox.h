#pragma once
#include <glm/glm.hpp>
#include "Model.h"
#include <vector>


class BoundingBox {
public:
	BoundingBox() = default;

	/// <summary>
	/// p1-p4 are top vertices of bounding box given clockwise
	/// p5-p8 are bottom verticies given clockwise
	/// </summary>
	BoundingBox(const glm::vec3& _p1, const glm::vec3& _p2, const glm::vec3& _p3, const glm::vec3& _p4, 
		const glm::vec3& _p5, const glm::vec3& _p6, const glm::vec3& _p7, const glm::vec3& _p8) :
		p1{ _p1 }, p2{ _p2 }, p3{ _p3 }, p4{ _p4 }, p5{ _p5 }, p6{ _p6 }, p7{ _p7 }, p8{ _p8 } {
		//p1-p4 are top vertices of bounding box given clockwise
		//p5-p8 are bottom verticies given clockwise
		std::vector<Vertex> vertices{ {p1},{p2},{p3},{p4},{p5},{p6},{p7},{p8} };
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

	glm::vec3 p1, p2, p3, p4, p5, p6, p7, p8;

private:
	Mesh boundingMesh;

};