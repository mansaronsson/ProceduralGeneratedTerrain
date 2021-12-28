#include "../header/CrystalMountains.h"


Crystal::Crystal(glm::vec3 pos, glm::vec3 dir) {

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	glm::vec3 color;

	// 1-4
	int nStems = 1;
	// for each stem
	float length = 5.0f;
	float width = 1.0f;
	// 3-8
	int nSides = 6;

	// vertices
	vertices.push_back({ pos });	// start

	glm::vec3 startAngle = glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f });

	for (int i = 0; i < nSides; ++i) {

		Vertex v{ pos + startAngle * width / 2.0f };
		v.position = glm::rotate(v.position, 2 * static_cast<float>(M_PI) * i / nSides, dir);

		// base
		vertices.push_back(v);
		vertices.push_back(v);
		vertices.push_back(v);

		// top
		v.position += dir * length * 0.8f;
		vertices.push_back(v);
		vertices.push_back(v);
		vertices.push_back(v);
	
		// end
		vertices.push_back({ pos + dir * length });
	}

	// normals
	vertices[0].normal = -dir;
	for (size_t i = 0; i < nSides; ++i) {

		// base
		vertices[i * 7 + 1].normal = -dir;

		// middle
		glm::vec3 e1 = vertices[i * 7 + 5].position - vertices[i * 7 + 2].position;
		glm::vec3 e2 = vertices[i * 7 + 9].position - vertices[i * 7 + 2].position;
		glm::vec3 n = glm::cross(e1, e2);

		vertices[i * 7 + 2].normal = n;
		vertices[i * 7 + 5].normal = n;
		vertices[i * 7 + 9].normal = n;
		vertices[i * 7 + 13].normal = n;

		// top
		e1 = vertices[i * 7 + 7].position - vertices[i * 7 + 4].position;
		e2 = vertices[i * 7 + 11].position - vertices[i * 7 + 4].position;
		n = glm::cross(e1, e2);

		vertices[i * 7 + 4].normal = n;
		vertices[i * 7 + 7].normal = n;
		vertices[i * 7 + 11].normal = n;
	}

	// indices
	for (unsigned int i = 0; i < nSides; ++i) {

		// base
		indices.push_back(0);
		indices.push_back(((i + 1) * 7 + 1) % (nSides * 7));
		indices.push_back(i * 7 + 1);

		// middle
		indices.push_back(i * 7 + 2);
		unsigned int temp = i * 7 + 9;
		indices.push_back(temp > nSides * 7 ? temp - nSides * 7 : temp);
		indices.push_back(i * 7 + 5);

		indices.push_back(i * 7 + 5);
		temp = i * 7 + 9;
		indices.push_back(temp > nSides * 7 ? temp - nSides * 7 : temp);
		temp = i * 7 + 13;
		indices.push_back(temp > nSides * 7 ? temp - nSides * 7 : temp);

		// top
		indices.push_back(i * 7 + 4);
		temp = i * 7 + 11;
		indices.push_back(temp > nSides * 7 ? temp - nSides * 7 : temp);
		indices.push_back(i * 7 + 7);
	}

	mesh = Mesh{ vertices, indices };
}

void Crystal::draw() {
	mesh.draw(GL_TRIANGLES);
}

