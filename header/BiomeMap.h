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

	/// <summary>
	/// Counts a number of biome sample points in an area around the given input position and returns a vector containing each biome's counter.
	/// </summary>
	/// <param name="xPos"></param>
	/// <param name="zPos"></param>
	/// <returns></returns>
	std::vector<unsigned short> countNeighbours(float xPos, float zPos) const;

	/// <summary>
	/// Updates the biome map based on the movement specified in the chunkChecker input parameter.
	/// </summary>
	/// <param name="cc"></param>
	void update(chunkChecker cc);

private:

	/// <summary>
	/// Returns the biome for the input position.
	/// </summary>
	/// <param name="xPos"></param>
	/// <param name="zPos"></param>
	/// <returns></returns>
	Biome generateBiome(float xPos, float zPos) const;

	const unsigned int m_gridSize;
	const float m_chunkWidth;
	const int m_samplesPerChunk;

	std::mutex mu;

	std::map<float, std::map<float, Biome>> m_map;
};