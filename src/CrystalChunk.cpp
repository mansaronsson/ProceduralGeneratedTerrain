#include "../header/CrystalChunk.h"


CrystalChunk::CrystalChunk(const glm::vec3& pos, const glm::vec3& dir) {
	
	float rand = abs(glm::simplex(pos * 2.0f));
	int nStems = static_cast<int>(1 + 6.0f * rand);

	glm::mat4 M;

	for (int stem = 0; stem < nStems; stem++)
	{
		glm::vec3 stemDir;
		if (nStems == 1 || (nStems > 3 && stem == 0)) {
			stemDir = dir;
		}
		else if (nStems > 3) {
			stemDir = glm::normalize(dir + glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			M = glm::rotate(static_cast<float>(M_PI) * 2 * (stem - 1) / (nStems - 1), dir);
			//stemDir = glm::rotate(stemDir, static_cast<float>(M_PI) * 2 * (stem-1) / (nStems-1), dir);
			stemDir = M * glm::vec4{ stemDir, 1.0f };
		}
		else {
			stemDir = glm::normalize(dir + glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f }));
			M = glm::rotate(static_cast<float>(M_PI) * 2 * stem / nStems, dir);
			//stemDir = glm::rotate(stemDir, static_cast<float>(M_PI) * 2 * stem / nStems, dir);
			stemDir = M * glm::vec4{ stemDir, 1.0f };
		}

		crystals.push_back(new Crystal{ pos, glm::vec3{stemDir} });
	}
}

CrystalChunk::~CrystalChunk()
{
	for (auto c : crystals) {
		delete c;
		c = nullptr;
	}
}

void CrystalChunk::bake()
{
	for (auto c : crystals) {
		c->bake();
	}
}

void CrystalChunk::draw() {
	for (auto c : crystals) {
		c->draw();
	}
}

CrystalChunk::Crystal::Crystal(const glm::vec3& pos, const glm::vec3& dir) {

	// 3-8
	float rand = abs(glm::simplex(pos * 3.0f));
	int nSides = static_cast<int>(3 + 7.0f * rand);
	// 0.05-0.15
	rand = abs(glm::simplex(pos * 4.0f));
	float baseWidth = 0.01f + 0.03f * rand;
	// 0.1-0.2
	rand = abs(glm::simplex(pos * 5.0f));
	float midWidth = 0.02f + 0.05f * rand;
	// 0.1-0.2
	rand = abs(glm::simplex(pos * 6.0f));
	float midLength = 0.02f + 0.08f * rand;
	// 0.5-0.8
	rand = abs(glm::simplex(pos * 7.0f));
	float topLength = 0.1f + 0.1f * rand;

	// vertices
	m_vertices.push_back({ pos });	// start

	glm::vec3 startAngle = glm::cross(dir, glm::vec3{ 1.0f, 0.0f, 0.0f });
	glm::mat4 M;

	for (int side = 0; side < nSides; ++side) {

		Vertex vb{ pos + startAngle * baseWidth / 2.0f };
		Vertex vm{ pos + startAngle * midWidth / 2.0f + dir * midLength };

		M = glm::translate(pos);
		M = glm::rotate(M, static_cast<float>(M_PI) * 2 * side / nSides, dir);
		M = glm::translate(M, -pos);

		vb.position = M * glm::vec4{ vb.position, 1.0f };
		vm.position = M * glm::vec4{ vm.position, 1.0f };

		// base
		m_vertices.push_back(vb);
		m_vertices.push_back(vb);
		m_vertices.push_back(vb);

		// mid
		m_vertices.push_back(vm);
		m_vertices.push_back(vm);
		m_vertices.push_back(vm);
		m_vertices.push_back(vm);
	
		// end
		m_vertices.push_back({ pos + dir * topLength });
	}

	// normals
	m_vertices[0].normal = glm::normalize(-dir);
	for (size_t i = 0; i < nSides; ++i) {

		// base
		m_vertices[i * 8 + 1].normal = glm::normalize(-dir);

		// middle
		glm::vec3 e1 = m_vertices[i * 8 + 6].position - m_vertices[i * 8 + 2].position;
		size_t temp = i * 8 + 10;
		glm::vec3 e2 = m_vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].position - m_vertices[i * 8 + 2].position;
		glm::vec3 n = glm::normalize(glm::cross(e2, e1));

		m_vertices[i * 8 + 2].normal = n;
		m_vertices[i * 8 + 6].normal = n;
		temp = i * 8 + 11;
		m_vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].normal = n;
		temp = i * 8 + 15;
		m_vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].normal = n;

		// top
		e1 = m_vertices[i * 8 + 8].position - m_vertices[i * 8 + 4].position;
		temp = i * 8 + 13;
		e2 = m_vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].position - m_vertices[i * 8 + 4].position;
		n = glm::normalize(glm::cross(e2, e1));

		m_vertices[i * 8 + 4].normal = n;
		m_vertices[i * 8 + 8].normal = n;
		temp = i * 8 + 13;
		m_vertices[temp > nSides * 8 ? temp - nSides * 8 : temp].normal = n;
	}

	// indices
	for (unsigned int i = 0; i < nSides; ++i) {

		// base
		m_indices.push_back(0);
		unsigned int temp = i * 8 + 9;
		m_indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		m_indices.push_back(i * 8 + 1);

		// middle
		m_indices.push_back(i * 8 + 2);
		temp = i * 8 + 11;
		m_indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		m_indices.push_back(i * 8 + 6);

		m_indices.push_back(i * 8 + 6);
		temp = i * 8 + 11;
		m_indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		temp = i * 8 + 15;
		m_indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);

		// top
		m_indices.push_back(i * 8 + 4);
		temp = i * 8 + 13;
		m_indices.push_back(temp > nSides * 8 ? temp - nSides * 8 : temp);
		m_indices.push_back(i * 8 + 8);
	}
}

CrystalChunk::Crystal::~Crystal()
{
	mesh.deleteMesh();
}

void CrystalChunk::Crystal::bake()
{
	mesh = Mesh{ m_vertices, m_indices };
}

void CrystalChunk::Crystal::draw() {
	mesh.draw(GL_TRIANGLES);
}

