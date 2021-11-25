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

		}
	}
	currentChunk = chunks[gridSize * gridSize / 2];
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
