#include "../header/CrystalChunk.h"


CrystalChunk::CrystalChunk(const glm::vec3& pos, const glm::vec3& dir) {
	
	// 1-5
	int nStems = 1;

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

CrystalChunk::~CrystalChunk()
{
	for (auto c : crystals) {
		delete c;
		c = nullptr;
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
	int nSides = 8;

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
		vertices.push_back(vm);
	
		// end
		vertices.push_back({ pos + dir * topLength });
	}

	// normals
	vertices[0].normal = -dir;
	for (size_t i = 0; i < nSides; ++i) {

		// base
		vertices[i * 8 + 1].normal = -dir;

		// middle
		glm::vec3 e1 = vertices[i * 8 + 6].position - vertices[i * 8 + 2].position;
		size_t temp = i * 8 + 10;
		glm::vec3 e2 = vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].position - vertices[i * 8 + 2].position;
		glm::vec3 n = glm::cross(e2, e1);

		vertices[i * 8 + 2].normal = n;
		vertices[i * 8 + 6].normal = n;
		temp = i * 8 + 11;
		vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].normal = n;
		temp = i * 8 + 15;
		vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].normal = n;

		// top
		e1 = vertices[i * 8 + 8].position - vertices[i * 8 + 4].position;
		temp = i * 8 + 13;
		e2 = vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].position - vertices[i * 8 + 4].position;
		n = glm::cross(e2, e1);

		vertices[i * 8 + 4].normal = n;
		vertices[i * 8 + 8].normal = n;
		temp = i * 8 + 13;
		vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].normal = n;
	}

	// indices
	for (unsigned int i = 0; i < nSides; ++i) {

		// base
		indices.push_back(0);
		unsigned int temp = i * 8 + 9;
		indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		indices.push_back(i * 8 + 1);

		// middle
		indices.push_back(i * 8 + 2);
		temp = i * 8 + 11;
		indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		indices.push_back(i * 8 + 6);

		indices.push_back(i * 8 + 6);
		temp = i * 8 + 11;
		indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		temp = i * 8 + 15;
		indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);

		// top
		indices.push_back(i * 8 + 4);
		temp = i * 8 + 13;
		indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		indices.push_back(i * 8 + 8);
	}

	mesh = Mesh{ vertices, indices };
}

CrystalChunk::Crystal::~Crystal()
{
	mesh.deleteMesh();
}

void CrystalChunk::Crystal::draw() {
	mesh.draw(GL_TRIANGLES);
}

