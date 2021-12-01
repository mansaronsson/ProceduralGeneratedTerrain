#include "..\header\ChunkHandler.h"
#include <iostream>

ChunkHandler::ChunkHandler(size_t _gridSize, size_t _nrVertices, float _spacing, float _yscale)
	: gridSize{ (_gridSize % 2 == 0 ? (_gridSize + 1) : _gridSize) }, nrVertices{ _nrVertices }, spacing{ _spacing }, yscale{ _yscale }, currentChunk{ nullptr }
{
	auto size = nrVertices;
	float width = (size  - 1) * spacing; //width of 1 chunk 

	for (int row = 0; row < gridSize; ++row) {
		float zpos = -width * (static_cast<float>(gridSize) / 2.0f) + row * width;
		for (int col = 0; col < gridSize; ++col) {
			float xpos = -width * (static_cast<float>(gridSize) / 2.0f) + col * width;

			chunks.push_back(new Chunk{ nrVertices, xpos, zpos, spacing, yscale, 2.5f });

		}
	}
	currentChunk = chunks[gridSize * gridSize / 2];
}

ChunkHandler::Chunk::Chunk(size_t _size, float xpos, float zpos, float _spacing, float yscale, float frequency) : size{ _size } {
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(size * size);
	indices.reserve(6 * (size - 2) * (size - 2) + 3 * size * 4 - 18); // se notes in lecture 6 

	//Need min and max height of this chunk to compute the bounding box
	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();


	for (int depth = 0; depth < size; ++depth)
	{
		for (int width = 0; width < size; ++width) {
			float x = xpos + width * _spacing;
			float z = zpos + depth * _spacing;

			/*** Apply noise to the height ie. y component using fbm ***/
			int octaves = 6;
			float noiseSum = 0.0f;
			float seed = 0.1f;
			float amplitude = 3.0f;
			float gain = 0.5;
			float lacunarity = 2.0;
			float freq = 0.09f;

			for (int i = 0; i < octaves; ++i) {
				noiseSum += amplitude * glm::perlin(glm::vec3((x+1) * freq, (z+1) * freq, seed));
				freq *= lacunarity;
				amplitude *= gain;
			}
			//float noiseY = glm::perlin(glm::vec3(x / frequency, z / frequency, 0.1f)); // last component seed 
			//noiseY *= yscale;
			//std::cout << "Sum:" << noiseSum << '\n';
			float noiseY = noiseSum;

			glm::vec3 pos{ x, noiseY, z };
			vertices.push_back({ pos });

			minY = minY > noiseY ? noiseY : minY;
			maxY = maxY < noiseY ? noiseY : maxY;

			//add indices to create triangle list
			if (width < size - 1 && depth < size - 1) {
				unsigned int i1, i2, i3, i4;
				i1 = index(width, depth); //current
				i2 = index(width, depth + 1); //bottom
				i3 = index(width + 1, depth + 1); //bottom right
				i4 = index(width + 1, depth); // right 

				/*
					i1--<--i4
					 |\    |
					 v \   ^
					 |  \  |
					 |   \ |
					i2-->--i3
				*/

				//left triangle diagonal from top left to bottom right 
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
				//right triangle
				indices.push_back(i1);
				indices.push_back(i3);
				indices.push_back(i4);
			}
		}
	}

	//compute normal by weighting all connected triangles
	for (int depth = 0; depth < size; ++depth)
	{
		for (int width = 0; width < size; ++width) {
			std::vector<glm::vec3> neigborhs;

			//neighbor flags 
			bool up, left, right, down;
			up = depth - 1 >= 0;
			down = depth + 1 < size;
			left = width - 1 >= 0;
			right = width + 1 < size;

			int counter{ 0 };
			glm::vec3 normal{ 0.0f, 0.0f, 0.0f };

			glm::vec3 v0 = vertices[index(width, depth)].position; //current
			//t1 & t6
			if (up && left)
			{
				glm::vec3 v1 = vertices[index(width - 1, depth - 1)].position; //nw
				glm::vec3 v2 = vertices[index(width - 1, depth)].position; //w
				glm::vec3 v3 = vertices[index(width, depth - 1)].position; //n
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				glm::vec3 e3 = v3 - v0;
				normal += glm::cross(e1, e2);
				normal += glm::cross(e3, e1);
				counter += 2;

			}
			//t2
			if (left && down) {
				glm::vec3 v1 = vertices[index(width - 1, depth)].position; //w
				glm::vec3 v2 = vertices[index(width, depth + 1)].position; //s
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				normal += glm::cross(e1, e2);
				++counter;
			}
			//t3 & t4
			if (down && right) {
				glm::vec3 v1 = vertices[index(width, depth + 1)].position; //s
				glm::vec3 v2 = vertices[index(width + 1, depth + 1)].position; //se
				glm::vec3 v3 = vertices[index(width + 1, depth)].position; //e
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				glm::vec3 e3 = v3 - v0;
				normal += glm::cross(e1, e2);
				normal += glm::cross(e3, e1);
				counter += 2;
			}
			//t5
			if (up && right) {
				glm::vec3 v1 = vertices[index(width + 1, depth)].position; //e
				glm::vec3 v2 = vertices[index(width + 1, depth - 1)].position; //ne
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				normal += glm::cross(e1, e2);
				++counter;
			}
			//if (counter == 1) {
			//	std::cout << "counter: " << counter << "at depth: " << depth << " width: " << width << '\n';
			//	std::cout << "x: " << normal.x << " y: " << normal.y << " z: " << normal.z << "\n\n";
			//}

			normal /= counter;
			normal = glm::normalize(normal);
			vertices[index(width, depth)].normal = normal;
		}
	}


	mesh = Mesh{ vertices, indices };

	//create boundingbox
	float minX = xpos;
	float maxX = xpos + (size - 1) * _spacing;
	float minZ = zpos;
	float maxZ = zpos + (size - 1) * _spacing;

	//Important theese are given in correct order -> see BoundingBox.h ctor
	glm::vec3 p1{ minX, maxY, minZ }, p2{ maxX, maxY, minZ }, p3{ maxX, maxY, maxZ }, p4{ minX, maxY, maxZ },
		p5{ minX, minY, minZ }, p6{ maxX, minY, minZ }, p7{ maxX, minY, maxZ }, p8{ minX, minY, maxZ };
	//
	//std::cout << glm::to_string(p1) << " " << glm::to_string(p2) << " " << glm::to_string(p3) << '\n'
	//	<< glm::to_string(p4) << " " << glm::to_string(p5) << " " << glm::to_string(p6) << '\n'
	//	<< glm::to_string(p7) << " " << glm::to_string(p8) << '\n' << '\n';

	boundingBox = BoundingBox{ p1, p2, p3, p4, p5, p6, p7, p8 };
}

chunkChecker ChunkHandler::Chunk::isInside(const glm::vec3& pos)
{
	//std::cout << glm::to_string(pos) << "\n";
	chunkChecker ch = inside;
	auto v1 = mesh.vertices[0].position; //first vertex in chunk
	auto v2 = mesh.vertices[mesh.vertices.size() - 1].position; // last vertex in chunk

	//Check if position is within chunk borders spanned by v1 and v2 in the x,z plane
	if (pos.z < v1.z) {
		ch = up;
		return ch;
	}
	if (pos.z > v2.z) {
		ch = down;
		return ch;
	}
	if (pos.x < v1.x) {
		ch = left;
		return ch;
	}
	if (pos.x > v2.x) {
		ch = right;
		return ch;
	}
	//Position is inside borders
	return ch;
}

std::pair<glm::vec3, glm::vec3> ChunkHandler::Chunk::computePN(const glm::vec3& n) const
{
	if (n.x > 0 && n.y > 0 && n.z > 0) { // + + +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p3, boundingBox.p5};
	}
	if (n.x > 0 && n.y > 0 && n.z < 0) { // + + -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p2, boundingBox.p8};
	}
	if (n.x > 0 && n.y < 0 && n.z > 0) { // + - +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p7, boundingBox.p1};
	}
	if (n.x > 0 && n.y < 0 && n.z < 0) { // + - - 
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p6, boundingBox.p4};
	}
	if (n.x < 0 && n.y > 0 && n.z > 0) { // - +  +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p4, boundingBox.p6};
	}
	if (n.x < 0 && n.y > 0 && n.z < 0) { // - + -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p1, boundingBox.p7};
	}
	if (n.x < 0 && n.y < 0 && n.z > 0) { // - - +
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p8, boundingBox.p2};
	}
	if (n.x < 0 && n.y < 0 && n.z < 0) { // - - -
		return std::pair<glm::vec3, glm::vec3>{boundingBox.p5, boundingBox.p3};
	}
}

void ChunkHandler::checkChunk(const glm::vec3& camPos)
{
	chunkChecker ch = currentChunk->isInside(camPos);
	if (ch != inside) {
		updateCurrentChunk(ch);
	}
}


// TODO: Fix the memory leaks this function creates
void ChunkHandler::updateCurrentChunk(chunkChecker ch)
{
	switch (ch)
	{
	case chunkChecker::up:

		for (size_t i = 0; i < gridSize*gridSize; ++i) {
			size_t j = gridSize * gridSize - 1 - i;
			// Delete bottom chunks
			if (j > gridSize * (gridSize - 1)) {
				delete chunks[j];
				chunks[j] = nullptr;
			}

			// Move pointers
			if (j >= gridSize) {
				chunks[j] = chunks[j - gridSize];
			}
			// Create new chunks above previous top chunks
			else {
				glm::vec3 pos = chunks[j]->getPostition();
				chunks[j] = new Chunk{ nrVertices, pos.x, pos.z - (nrVertices-1) * spacing, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "up\n";
		break;

	case chunkChecker::down:

		for (size_t i = 0; i < gridSize * gridSize; ++i) {
			
			// Delete top chunks
			if (i < gridSize) {
				delete chunks[i];
				chunks[i] = nullptr;
			}

			// Move pointers
			if (i < gridSize * (gridSize - 1)) {
				chunks[i] = chunks[i + gridSize];
			}
			// Create new chunks below previous bot chunks
			else {
				glm::vec3 pos = chunks[i]->getPostition();
				chunks[i] = new Chunk{ nrVertices, pos.x, pos.z + (nrVertices - 1) * spacing, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "down\n";
		break;

	case chunkChecker::left:

		for (size_t i = 0; i < gridSize * gridSize; ++i) {
			
			size_t j = gridSize * gridSize - 1 - i;

			// Delete right chunks
			if (j % gridSize == gridSize - 1) {
				delete chunks[j];
				chunks[j] = nullptr;
			}

			// Move pointers
			if (j % gridSize != 0) {
				chunks[j] = chunks[j - 1];
			}
			// Create new chunks to the left of previous left chunks
			else {
				glm::vec3 pos = chunks[j]->getPostition();
				chunks[j] = new Chunk{ nrVertices, pos.x - (nrVertices - 1) * spacing, pos.z, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "left\n";
		break;

	case chunkChecker::right:

		for (size_t i = 0; i < gridSize * gridSize; ++i) {

			// Delete left chunks
			if (i % gridSize == 0) {
				delete chunks[i];
				chunks[i] = nullptr;
			}

			// Move pointers
			if (i % gridSize != gridSize - 1) {
				chunks[i] = chunks[i + 1];
			}
			// Create new chunks to the right of previous right chunks
			else {
				glm::vec3 pos = chunks[i]->getPostition();
				chunks[i] = new Chunk{ nrVertices, pos.x + (nrVertices - 1) * spacing, pos.z, spacing, yscale, 2.5f };
			}
		}

		currentChunk = chunks[gridSize * gridSize / 2];

		std::cout << "right\n";
		break;

	default:
		break;
	}
}

void ChunkHandler::cullTerrainChunk(const std::vector<CameraPlane>& cameraPlanes)
{
	for (Chunk* chunk : chunks) {
		bool outside = false, intersects = false;


		for (const CameraPlane& plane : cameraPlanes) {
			auto [p, n] = chunk->computePN(plane.normal);
			float a = plane.evaluatePlane(n);
			if (a > 0) // outside 
			{
				//chunk->drawChunk = false;
				outside = true;
				//std::cout << "Culling chunk \n";
				break;
			}

			float b = plane.evaluatePlane(p);
			if (b > 0) // intersects
				intersects = true;
		}

		//bounding box intersect, not nessesary for us?
		//if (intersects)
		//	return;

		//bounding box is inside -> draw object
		chunk->drawChunk = !outside;
	}
}

