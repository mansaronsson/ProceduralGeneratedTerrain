#include "../header/BiomeMap.h"


BiomeMap::BiomeMap(float xPos, float zPos, unsigned int gridSize, float chunkWidth, int samplesPerChunk) 
	: m_gridSize{ gridSize }, m_chunkWidth{ chunkWidth }, m_samplesPerChunk{ samplesPerChunk } {

	float mapWidth = (gridSize + 1) / 2 * chunkWidth; // Biomemap is a bit wider than the rendered chunks

	for (float x = xPos - mapWidth; x < xPos + mapWidth; x += chunkWidth / samplesPerChunk) {
		for (float z = zPos - mapWidth; z < zPos + mapWidth; z += chunkWidth / samplesPerChunk) {
		
			m_map[x][z] = generateBiome(x, z);
		}	
	}
}

std::vector<unsigned short> BiomeMap::countNeighbours(float xPos, float zPos) const {

	std::vector<unsigned short> result(nBiomes, 0);
	float searchDistance = m_chunkWidth / 3.0f;

	for (auto x = m_map.lower_bound(xPos - searchDistance); x != m_map.upper_bound(xPos + searchDistance); ++x) {
		for (auto z = x->second.lower_bound(zPos - searchDistance); z != x->second.upper_bound(zPos + searchDistance); ++z) {
			++result[static_cast<size_t>(z->second)];
		}
	}
 
	return result;
}

Biome BiomeMap::generateBiome(float xPos, float zPos) const
{
	float freq{ 0.0031f };
	float seed{ 1337.0f };
	float amp{ 2.0f };

	float temperature = amp * glm::simplex(glm::vec3(xPos * freq, zPos * freq, seed));
	float humidity = amp * glm::simplex(glm::vec3(xPos * freq, zPos * freq, seed * 2));


	if (temperature > 0.2 && humidity < -0.2) {
		return desert;
	}
	else if (temperature < -0.3 && humidity > 0.3) {
		return crystalMountain;
	}
	return plains;
}

void BiomeMap::update(chunkChecker cc) {
	std::lock_guard<std::mutex> lock(mu);	// Thread safe

	float step = m_chunkWidth / m_samplesPerChunk;
	
	float xStart, zStart, zEnd;
	auto xit = m_map.begin();
	auto zit = xit->second.begin();
	switch (cc) {
		case up:
			for (; xit != m_map.end(); ++xit) {

				zStart = zit->first;
				for (float z = zStart - step; z > zStart - 2 * m_chunkWidth; z -= step) {
					xit->second[z] = generateBiome(xit->first, z);
				}
			}

			break;
		case down:
			for (; xit != m_map.end(); ++xit) {

				zit = xit->second.end();
				--zit;
				zStart = zit->first;
				for (float z = zStart + step; z < zStart + 2 * m_chunkWidth; z += step) {
					xit->second[z] = generateBiome(xit->first, z);
				}
			}

			break;
		case left:
			
			xStart = xit->first;
			zStart = zit->first;
			zit = xit->second.end();
			--zit;
			zEnd = zit->first;
			for (float x = xStart - step; x > xStart - 2 * m_chunkWidth; x -= step) {
				for (float z = zStart; z <= zEnd; z += step) {

					m_map[x][z] = generateBiome(x, z);
				}
			}
			
			break;
		case right:

			xit = m_map.end();
			--xit;
			zit = xit->second.begin();

			xStart = xit->first;
			zStart = zit->first;
			zit = xit->second.end();
			--zit;
			zEnd = zit->first;
			for (float x = xStart + step; x < xStart + 2 * m_chunkWidth; x += step) {
				for (float z = zStart; z <= zEnd; z += step) {

					m_map[x][z] = generateBiome(x, z);
				}
			}

			break;
		default:
			throw std::invalid_argument("invalid value of input chunkChecker");
	}
}