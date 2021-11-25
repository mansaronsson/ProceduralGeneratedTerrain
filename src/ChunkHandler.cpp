#include "..\header\ChunkHandler.h"
#include <iostream>

ChunkHandler::ChunkHandler(size_t nrVertices, float spacing, float yscale) : currentChunk{ nullptr }
{
	auto size = nrVertices;
	float width = (size  - 1) * spacing;

	for (int row = 0; row < chunkGridSize; ++row) {
		float zpos = width * 1.5 - row * width;
		for (int col = 0; col < chunkGridSize; ++col) {
			float xpos = -width * 1.5 + col * width;

			chunks.push_back(new Chunk{ nrVertices, xpos, zpos, spacing, yscale, 2.5 });

		}
	}
	currentChunk = chunks[4];
}

chunkChecker ChunkHandler::Chunk::isInside(const glm::vec3& pos)
{
	chunkChecker ch = inside;
	auto v1 = mesh.vertices[0].position; //first vertex in chunk
	auto v2 = mesh.vertices[mesh.vertices.size() - 1].position; // last vertex in chunk

	//Check if position is within chunk borders spanned by v1 and v2 in the x,z plane
	if (pos.x < v1.x) {
		ch = left;
		return ch;
	}
	if (pos.z > v1.z) {
		ch = up;
		return ch;
	}
	if (pos.x > v2.x) {
		ch = right;
		return ch;
	}
	if (pos.z < v2.z) {
		ch = down;
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

void ChunkHandler::updateCurrentChunk(chunkChecker ch)
{
	//TODO: generate new chunk row or column depending on direction we moved into, and shift all pointers
	//delete row / column that moved further away from
	switch (ch)
	{
	case chunkChecker::up:
		break;
	case chunkChecker::down:
		break;
	case chunkChecker::left:
		break;
	case chunkChecker::right:
		break;
	default:
		break;
	}
}
