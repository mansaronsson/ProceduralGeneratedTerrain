#pragma once

#include <glm/gtc/noise.hpp>

#include <map>
#include <vector>
#include <stdexcept> 
#include <mutex>

#include "enums.h"


class BiomeMap {
public:
	BiomeMap(float xPos, float zPos, unsigned int gridSize, float chunkWidth, int samplesPerChunk);

	std::vector<unsigned short> countNeighbours(float xPos, float zPos) const;
	void update(chunkChecker cc);

private:
	Biome generateBiome(float xPos, float zPos) const;

	const unsigned int m_gridSize;
	const float m_chunkWidth;
	const int m_samplesPerChunk;

	std::mutex mu;

	std::map<float, std::map<float, Biome>> m_map;
};