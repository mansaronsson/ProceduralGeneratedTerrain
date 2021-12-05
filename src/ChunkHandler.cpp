#include "..\header\ChunkHandler.h"
#include <iostream>

ChunkHandler::ChunkHandler(size_t _gridSize, size_t _nrVertices, float _spacing, float _yscale)
	: gridSize{ _gridSize }, nrVertices{ _nrVertices }, spacing{ _spacing }, yscale{ _yscale }, currentChunk{ nullptr }
{
	auto size = nrVertices;
	float width = (size  - 1) * spacing;

	for (int row = 0; row < gridSize; ++row) {
		float zpos = -width * (static_cast<float>(gridSize) / 2.0f) + row * width;
		for (int col = 0; col < gridSize; ++col) {
			float xpos = -width * (static_cast<float>(gridSize) / 2.0f) + col * width;

			chunks.push_back(new Chunk{ nrVertices, xpos, zpos, spacing, yscale, 2.5f });
			chunks.back()->id = index(col, row);
		}
	}
	currentChunk = chunks[gridSize * gridSize / 2];
}

ChunkHandler::Chunk::Chunk(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<glm::vec3>& bBox, size_t _size)
	: size{_size}
{

	mesh = Mesh{ vertices, indices };
	boundingBox = BoundingBox{ bBox };
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

chunkChecker ChunkHandler::checkChunk(const glm::vec3& camPos)
{
	return currentChunk->isInside(camPos);
}

void ChunkHandler::generateChunk(chunkChecker cc, size_t id)
{
	const float frequency{ 2.5f };	// Should be changed in later versions
	
	glm::vec3 prevPos = chunks[id]->getPostition();
	float posX;
	float posZ;

	switch (cc) {
	case chunkChecker::up:
		posX = prevPos.x;
		posZ = prevPos.z - (nrVertices - 1) * spacing;
		break;
	case chunkChecker::down:
		posX = prevPos.x;
		posZ = prevPos.z + (nrVertices - 1) * spacing;
		break;
	case chunkChecker::left:
		posX = prevPos.x - (nrVertices - 1) * spacing;
		posZ = prevPos.z;
		break;
	case chunkChecker::right:
		posX = prevPos.x + (nrVertices - 1) * spacing;
		posZ = prevPos.z;
		break;
	default:
		break;
	}

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	vertices.reserve(nrVertices * nrVertices);
	indices.reserve(6 * (nrVertices - 2) * (nrVertices - 2) + 3 * nrVertices * 4 - 18); // se notes in lecture 6 

	float minY = std::numeric_limits<float>::max();
	float maxY = std::numeric_limits<float>::min();

	for (int depth = 0; depth < nrVertices; ++depth)
	{
		for (int width = 0; width < nrVertices; ++width) {
			float x = posX + width * spacing;
			float z = posZ + depth * spacing;
			float noiseY = glm::perlin(glm::vec3(x / frequency, z / frequency, 0.1f)); // last component seed 
			noiseY *= yscale;


			vertices.push_back({ { x, noiseY, z } });

			minY = minY > noiseY ? noiseY : minY;
			maxY = maxY < noiseY ? noiseY : maxY;


			unsigned int i1, i2, i3, i4;
			if (width < nrVertices - 1 && depth < nrVertices - 1) {
				i1 = chunkIndex(width, depth); //current
				i2 = chunkIndex(width, depth + 1); //bottom
				i3 = chunkIndex(width + 1, depth + 1); //bottom right
				i4 = chunkIndex(width + 1, depth); // right 

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
	for (int depth = 0; depth < nrVertices; ++depth)
	{
		for (int width = 0; width < nrVertices; ++width) {

			//neighbor flags 
			bool up, left, right, down;
			up = depth - 1 >= 0;
			down = depth + 1 < nrVertices;
			left = width - 1 >= 0;
			right = width + 1 < nrVertices;

			int counter{ 0 };
			glm::vec3 normal{ 0.0f,0.0f,0.0f };

			glm::vec3 v0 = vertices[chunkIndex(width, depth)].position; //current
			//t1 & t6
			if (up && left)
			{
				glm::vec3 v1 = vertices[chunkIndex(width - 1, depth - 1)].position; //nw
				glm::vec3 v2 = vertices[chunkIndex(width - 1, depth)].position; //w
				glm::vec3 v3 = vertices[chunkIndex(width, depth - 1)].position; //n
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				glm::vec3 e3 = v3 - v0;
				normal += glm::cross(e1, e2);
				normal += glm::cross(e3, e1);
				counter += 2;

			}
			//t2
			if (left && down) {
				glm::vec3 v1 = vertices[chunkIndex(width - 1, depth)].position; //w
				glm::vec3 v2 = vertices[chunkIndex(width, depth + 1)].position; //s
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				normal += glm::cross(e1, e2);
				++counter;
			}
			//t3 & t4
			if (down && right) {
				glm::vec3 v1 = vertices[chunkIndex(width, depth + 1)].position; //s
				glm::vec3 v2 = vertices[chunkIndex(width + 1, depth + 1)].position; //se
				glm::vec3 v3 = vertices[chunkIndex(width + 1, depth)].position; //e
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				glm::vec3 e3 = v3 - v0;
				normal += glm::cross(e1, e2);
				normal += glm::cross(e3, e1);
				counter += 2;
			}
			//t5
			if (up && right) {
				glm::vec3 v1 = vertices[chunkIndex(width + 1, depth)].position; //e
				glm::vec3 v2 = vertices[chunkIndex(width + 1, depth - 1)].position; //ne
				glm::vec3 e1 = v1 - v0;
				glm::vec3 e2 = v2 - v0;
				normal += glm::cross(e1, e2);
				++counter;
			}

			normal /= counter;
			normal = glm::normalize(normal);
			vertices[chunkIndex(width, depth)].normal = normal;
		}
	}

	//create boundingbox
	float minX = posX;
	float maxX = posX + (nrVertices - 1) * spacing;
	float minZ = posZ;
	float maxZ = posZ + (nrVertices - 1) * spacing;

	std::vector<glm::vec3> bBox{ { minX, maxY, minZ }, { maxX, maxY, minZ }, { maxX, maxY, maxZ }, { minX, maxY, maxZ },
		{ minX, minY, minZ }, { maxX, minY, minZ }, { maxX, minY, maxZ }, { minX, minY, maxZ } };

	renderQ.push({vertices, indices, bBox, cc, id});
}


// TODO: Fix the memory leaks this function creates
void ChunkHandler::updateChunks(const glm::vec3& camPos)
{
	chunkChecker cc{ this->checkChunk(camPos) };

	std::vector<size_t> ids;
	switch (cc)
	{
	case chunkChecker::inside:
		break;

	case chunkChecker::up:

		currentChunk = chunks[currentChunk->id - gridSize];
		moveQ.push(cc);
		break;

	case chunkChecker::down:

		currentChunk = chunks[currentChunk->id + gridSize];
		moveQ.push(cc);
		break;

	case chunkChecker::left:

		currentChunk = chunks[currentChunk->id - 1];
		moveQ.push(cc);
		break;

	case chunkChecker::right:

		currentChunk = chunks[currentChunk->id + 1];
		moveQ.push(cc);
		break;
	}

	// MoveQ evaluator
	if (!moveQ.empty() && renderCounter == gridSize)
	{
		renderCounter = 0;
		std::vector<size_t> ids;

		chunkChecker cc = moveQ.front();
		moveQ.pop();

		switch (cc)
		{
		case chunkChecker::up:
			for (size_t id = 0; id < gridSize; ++id) {
				ids.push_back(id);
			}
			break;

		case chunkChecker::down:
			for (size_t id = gridSize * gridSize; id > gridSize * (gridSize - 1); --id) {
				ids.push_back(id - 1);
			}
			break;

		case chunkChecker::left:
			for (size_t id = 0; id < gridSize; ++id) {
				ids.push_back(id * gridSize);
			}
			break;

		case chunkChecker::right:
			for (size_t id = 0; id < gridSize; ++id) {
				ids.push_back(id * gridSize + gridSize - 1);
			}
			break;
		}

		// Multithreading
		for (auto id : ids)
		{
			std::thread t(&ChunkHandler::generateChunk, this, cc, id);	
			t.detach();
		}
	}


	while (!renderQ.empty()) {

		// 0: Vertex, 1: unsigned int, 2: vec3, 3: chunkChecker, 4: size_t
		chunkInfo ci = renderQ.front();
		renderQ.pop();
		++renderCounter;

		size_t id = std::get<size_t>(ci);
		Chunk* newChunk = new Chunk{ std::get<0>(ci), std::get<1>(ci), std::get<2>(ci), nrVertices };
		newChunk->id = id;
		Chunk* temp;

		switch (std::get<chunkChecker>(ci))
		{
		case chunkChecker::up:
			temp = chunks[id + gridSize * (gridSize - 1)];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id + gridSize * (gridSize - i)] = chunks[id + gridSize * (gridSize - i - 1)];
				chunks[id + gridSize * (gridSize - i)]->id += gridSize;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::down:
			temp = chunks[id - gridSize * (gridSize - 1)];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id - gridSize * (gridSize - i)] = chunks[id - gridSize * (gridSize - i - 1)];
				chunks[id - gridSize * (gridSize - i)]->id -= gridSize;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::left:
			temp = chunks[id + gridSize - 1];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id + gridSize - i] = chunks[id + gridSize - i - 1];
				++chunks[id + gridSize - i]->id;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		case chunkChecker::right:
			temp = chunks[id - gridSize + 1];	// Chunk to be deleted

			for (size_t i = 1; i < gridSize; ++i) {

				chunks[id - gridSize + i] = chunks[id - gridSize + i + 1];
				--chunks[id - gridSize + i]->id;
			}

			chunks[id] = newChunk;
			delete temp;
			temp = nullptr;

			break;

		default:
			break;
		}


	}
}
