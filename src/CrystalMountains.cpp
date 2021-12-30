#include "../header/CrystalMountains.h"


CrystalChunk::CrystalChunk(const glm::vec3& pos, const glm::vec3& dir) {
	
	// 1-4
	int nStems = 4;

	for (int stem = 0; stem < nStems; stem++)
	{
		glm::vec3 stemDir;
		if (nStems == 1 || (nStems > 3 && stem == 0)) {
			stemDir = dir;
		}
		else if (nStems > 3) {
			stemDir = glm::normalize(dir + glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			stemDir = glm::rotate(stemDir, static_cast<float>(M_PI) * 2 * (stem-1) / (nStems-1), dir);
		}
		else {
			stemDir = glm::normalize(dir + glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			stemDir = glm::rotate(stemDir, static_cast<float>(M_PI) * 2 * stem / nStems, dir);
		}

		crystals.push_back(new Crystal{pos, stemDir});
	}
}

void CrystalChunk::draw() {
	for (auto c : crystals) {
		c->draw();
	}
}

CrystalChunk::Crystal::Crystal(const glm::vec3& pos, const glm::vec3& dir) {

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	glm::vec3 color;

	// for each stem
	float baseWidth = 1.0f;
	float midWidth = 2.0f;

	float midLength = 3.0f;
	float topLength = 5.0f;

	// 3-8
	int nSides = 6;

	// vertices
	vertices.push_back({ pos });	// start

	glm::vec3 startAngle = glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f });

	for (int side = 0; side < nSides; ++side) {

		Vertex vb{ pos + startAngle * baseWidth / 2.0f };
		Vertex vm{ pos + startAngle * midWidth / 2.0f + dir * midLength };
		vb.position = glm::rotate(vb.position, static_cast<float>(M_PI) * 2 * side / nSides, dir);
		vm.position = glm::rotate(vm.position, static_cast<float>(M_PI) * 2 * side / nSides, dir);

		// base
		vertices.push_back(vb);
		vertices.push_back(vb);
		vertices.push_back(vb);

		// mid
		vertices.push_back(vm);
		vertices.push_back(vm);
		vertices.push_back(vm);
	
		// end
		vertices.push_back({ pos + dir * topLength });
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

void CrystalChunk::Crystal::draw() {
	mesh.draw(GL_TRIANGLES);
}

