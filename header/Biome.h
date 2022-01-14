#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <string>
#include "Mesh.h"
#include <iostream>
#include <map>

const std::string temperatureString[2]{ "cold", "hot" };
const std::string moistureString[2]{ "wet", "dry" };
const std::string biomeString[4]{ "ice", "tundra", " woodland", "desert" };
constexpr glm::vec3 biomeColors[4]{ glm::vec3{0.8f, 1.0f, 1.0f}, glm::vec3{0.3f, 1.0f, 0.7f}, glm::vec3{0.15f, 0.5f, 0.08f}, glm::vec3{1.0f, 0.7f, 0.1f} };
constexpr glm::vec3 tempcolors[2]{ glm::vec3{0.0f, 0.5f, 1.0f}, glm::vec3{1.0f, 0.1f, 0.0f} }; //cold, hot
constexpr glm::vec3 moistcolors[2]{ glm::vec3{0.0f, 0.5f, 1.0f}, glm::vec3{0.52f, 0.31f, 0.0f} }; //wet, dry

enum BiomeType {
	ice, tundra, woodland, desert
};

class Biome {
public:
	Biome(float hot = 0.5f, float wet = 0.5f) : heatMap{ NoiseMap{ 1, 1.0f, 0.5f, 2.0f, 0.007f, 132.153f } }, moistMap{ NoiseMap{1, 1.0f, 0.5f, 2.0f, 0.008f, 1.137f} }, COLDLIMIT{ hot }, WETLIMIT{ wet }{
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				biomepairs.push_back(std::make_pair(centerPoints[i][j], biomeTable[i][j]));
			}
		}
	}

	void setColorFromBiome(Vertex& v, float x, float z) const {
		float heatValue = 0.0f, moistValue = 0.0f;
		BiomeType biome = getBiomeAtPosition(x, z, &heatValue, &moistValue);

		//Interpolate color
		auto moistureColor = glm::mix(moistcolors[0], moistcolors[1], moistValue);
		auto temperatureColor = glm::mix(tempcolors[0], tempcolors[1], heatValue);

		//Set colors
		v.biomecolor = biomeColors[static_cast<int>(biome)];
		v.heatcolor = temperatureColor;
		v.moistcolor = moistureColor;
	}

	BiomeType getBiomeAtPosition(float x, float z, float* outHeat = nullptr, float* outMoist = nullptr) const {
		float heatValue = heatMap.evaluateWeather(x, z);
		float moistValue = moistMap.evaluateWeather(x, z);

		Temperature heat = getTemperatureAtValue(heatValue);
		Moisture moist = getMoistureAtValue(moistValue);

		if (outHeat)
			*outHeat = heatValue;
		if (outMoist)
			*outMoist = moistValue;

		return getBiome(heat, moist);
	}

	glm::vec2 computeWeatherValues(float x, float z) const {
		return glm::vec2{ heatMap.evaluateWeather(x,z), moistMap.evaluateWeather(x,z) };
	}

	glm::vec3 computeVertexPointFromBiomes(float x, float z, float* minY = nullptr, float* maxY = nullptr) const {
		float heatValue = 0.0f, moistValue = 0.0f;
		BiomeType biome = getBiomeAtPosition(x, z, &heatValue, &moistValue);

		/* Compute weights by takning the distance (inverse) to a predefined biomepoint  */
		float weightSum{ 0.0f };
		glm::vec3 sum{ 0.0f };
		for (auto b : biomepairs) {
			float d = glm::distance(glm::vec2{ heatValue, moistValue }, b.first);
			float weight = 1.0f / (d * d * d);
			weightSum += weight;
			sum += weight * computePositionInBiome(b.second, x, z);
			//std::cout << weight << " ";
			
		}
		sum /= weightSum;
		//std::cout << std::endl;

		//std::cout << "biome at cold, wet " << getBiome(Temperature::cold, Moisture::wet) << " "
		//	<< "biome at cold, dry " << getBiome(Temperature::cold, Moisture::dry) << " "
		//	<< "biome at hot, wet " << getBiome(Temperature::hot, Moisture::wet) << " "
		//	<< "biome at hot, dry " << getBiome(Temperature::hot, Moisture::dry) << " \n\n";

		/* Checking nearby biomes in woorld coordinates */
		//auto weights = computeWeights(x, z, 5.0f, 50);
		////std::cout << weights.x << " " << weights.y << " " << weights.z << " " << weights.w << "\n\n";
		//auto p1 = computePositionInBiome(BiomeType::ice, x, z);
		//auto p2 = computePositionInBiome(BiomeType::tundra, x, z);
		//auto p3 = computePositionInBiome(BiomeType::woodland, x, z);
		//auto p4 = computePositionInBiome(BiomeType::desert, x, z);

		//auto worldSum = weights.x * p1 + weights.y * p2 + weights.z * p3 + weights.w * p4;

		
		/* average weighted position of all 4 nearby biomes - to their respective edges in moisture / heat space */
		//float epsilon = heatValue > 0 ? 1e-5 : -1e-5;
		//float distanceHeat = heatValue - COLDLIMIT;
		//float distanceMoist = moistValue - WETLIMIT;
		//float threshold = 0.1f;
		//if (std::abs(distanceHeat) < threshold)
		//{
		//	//compute linear interpolation between current biome and other biom
		//}
		//Temperature newTemp = getTemperatureAtValue(heatValue - distanceHeat);
		////epsilon = moistValue > 0 ? 1e-5 : -1e-5;
		//Moisture newMoist = getMoistureAtValue(moistValue - distanceMoist);
		//BiomeType bio1 = getBiome(newTemp, moist);
		//BiomeType bio2 = getBiome(heat, newMoist);
		//BiomeType bio3 = getBiome(newTemp, newMoist);

		//float distbio3 = std::sqrtf(distanceHeat * distanceHeat + distanceMoist * distanceMoist);

		//float w1 = 0.5f - std::abs(distanceHeat);
		//float w2 = 0.5f - std::abs(distanceMoist);
		//float w3 = (std::sqrtf(0.5f) - distbio3) / std::sqrtf(0.5f) / 2.0f;
		//float w4 = 2.0f - w1 - w2 - w3;
		////std::cout << " sacle moistmap " << scl << '\n';
		////std::cout << "sum: " << w1 + w2 + w3 + w4 << " w1 " << w1 << " w2 " << w2 << " w3 " << w3 << " w4 " << w4 << std::endl;
		//
		////std::cout << biome << " " << bio1 << " " << bio2 << " " << bio3 << std::endl;
		////std::cout << "dist heat " << std::abs(distanceHeat) << " dist moist " << std::abs(distanceMoist) << " distance diagonal " << distbio3 << std::endl;

		////compute position in different biomes
		//auto p1 = computePositionInBiome(bio1, x, z);
		//auto p2 = computePositionInBiome(bio2, x, z);
		//auto p3 = computePositionInBiome(bio3, x, z);
		//auto p4 = computePositionInBiome(biome, x, z);

		//auto sum = (p1 * w1 + p2 * w2 + p3 * w3 + p4 * w4) / 4.0f ;
		
		/* Try and create triangle around point and compute weights using barycentric coordinates */
	
		//auto coefs = findNearestCentroids(glm::vec2{ heatValue, moistValue });
		//glm::vec3 newSum{ 0.0f };
		//for (auto p : coefs) {
		//	auto pos = computePositionInBiome(p.second, x, z);
		//	newSum += pos * p.first;
		//}
		//newSum /= coefs.size();
		
		//REMOVE ME 
		//sum = p4;

		float noiseY = sum.y;
	
		float groundlevel = -1.51f;
		if (noiseY < groundlevel)
			noiseY = groundlevel;

		if (minY != nullptr && maxY != nullptr) {
			*minY = *minY > noiseY ? noiseY : *minY;
			*maxY = *maxY < noiseY ? noiseY : *maxY;
		}

		glm::vec3 pos{ x, noiseY, z };

		//std::cout << "coeffs: " << coefs.x << " " << coefs.y << " " << coefs.z << '\n';
		//glm::vec2 p11{ 0.45, 0.45 };
		//glm::vec2 p22{ 0.45, 0.15 };
		//glm::vec2 p33{ 0.8, 0.15 };
		//auto coeffs = CartesianToBarycentricCoordinates(0.45f, 0.45f, p11, p22, p33);

		return pos;
	}

private:

	struct NoiseMap {
		//Based on https://thebookofshaders.com/13/
#include <glm/gtc/noise.hpp>
		/// <summary>
		/// 
		/// </summary>
		/// <param name="_octaves">octaves</param>
		/// <param name="_amp">amplitude</param>
		/// <param name="_gain">gain</param>
		/// <param name="_lacu">lacunarity</param>
		/// <param name="_freq">frequency</param>
		/// <param name="_seed">seed</param>
		NoiseMap(int _octaves, float _amp, float _gain, float _lacu, float _freq, float _seed) :
			octaves{ _octaves }, amplitude{ _amp }, gain{ _gain }, lacunarity{ _lacu }, frequency{ _freq }, seed{ _seed } {}
		NoiseMap() = default;

		int octaves;
		float amplitude;
		float gain;
		float lacunarity;
		float frequency;
		float seed;

		/// <summary>
		/// Evaluate height using simplex, return range [-1,1]
		/// </summary>
		float evaluate(float x, float z) const {
			float noiseSum = 0.0f;
			float _frequency = frequency;
			float _amplitude = amplitude;

			for (int i = 0; i < octaves; ++i) {
				noiseSum += _amplitude * glm::simplex(glm::vec3((x + 1) * _frequency, (z + 1) * _frequency, seed));
				//noiseSum += _amplitude * scaledPerlin(glm::vec3((x + 1) * _frequency, (z + 1) * _frequency, seed));

				_frequency *= lacunarity;
				_amplitude *= gain;
			}

			return noiseSum;
		}
		/// <summary>
		/// Evaluate weather, return range [0,1]
		/// </summary>
		float evaluateWeather(float x, float z) const {
			float noiseSum = 0.0f;
			float _frequency = frequency;
			float _amplitude = amplitude;

			for (int i = 0; i < octaves; ++i) {
				noiseSum += _amplitude * glm::simplex(glm::vec3((x + 1) * _frequency, (z + 1) * _frequency, seed));

				_frequency *= lacunarity;
				_amplitude *= gain;
			}
			//noiseSum = std::clamp(noiseSum, 0.0f, 1.0f);
			float scl = scale();
			noiseSum = map(noiseSum, -scl, scl, 0, 1);

			return noiseSum;
		}
		/// <summary>
		/// Map the value low1 - high1 to new range low2 - high2
		/// </summary>
		float map(float value, float low1, float high1, float low2, float high2) const {
			return low2 + (value - low1) * (high2 - low2) / (high1 - low1);
		}

		float scale() const {
			float sum = 0.0f;
			float _amplitude = amplitude;

			for (int i = 0; i < octaves; ++i) {
				sum += _amplitude;
				_amplitude *= gain;
			}
			return sum;
		}

		//rescale perlin [-1, 1] to [0, 1]
		float scaledPerlin(const glm::vec3& v) const {
			return glm::perlin(v) / 2.0f + 0.5f;
		}
	}; 
	/*** End of NoiseMap class ***/


	enum class Temperature
	{
		cold, hot
	};

	enum class Moisture {
		wet, dry
	};

	/// <summary>
	/// Evaluate the height value for a specific biome 
	/// </summary>
	glm::vec3 computePositionInBiome(BiomeType biome, float x, float z) const {
		NoiseMap mapBiome;
		NoiseMap tetonics{ 2, 3.3f, 0.5f, 2.0f, 0.04f, 182.327f };
		float scale = 1.0f;
		switch (biome)
		{
		case ice:
			mapBiome = NoiseMap{ 2, 0.3f * (1/scale), 0.5f, 2.0f, 0.03f * scale, 0.15f };
			break;
		case tundra:
			mapBiome = NoiseMap{ 4, 2.0f * (1/scale), 0.5f, 2.0f, 0.09f * scale, 0.1f };
			break;
		case woodland:
			mapBiome = NoiseMap{ 3, 4.3f * (1/scale), 0.5f, 2.0f, 0.13f * scale, 13.64f };
			break;
		case desert:
			mapBiome = NoiseMap{ 2, 1.5f * (1/scale), 0.5f, 2.0f, 0.02f * scale, 27.382f };
			break;
		default:
			std::cout << " Should never occur - forgot to add new biome type? \n";
			break;
		}
		float baseline = tetonics.evaluate(x, z);
		float y = mapBiome.evaluate(x, z) + baseline;
		return glm::vec3{ x, y, z };
	}

	Temperature getTemperatureAtValue(float val) const {
		if (val <= COLDLIMIT)
			return Temperature::cold;
		else
			return Temperature::hot;
	}
	Moisture getMoistureAtValue(float val) const {
		if (val <= WETLIMIT)
			return Moisture::wet;
		else
			return Moisture::dry;
	}

	BiomeType getBiome(Temperature temp, Moisture moist) const {
		return biomeTable[static_cast<int>(temp)][static_cast<int>(moist)];
	}

	const float COLDLIMIT, WETLIMIT;

	NoiseMap heatMap;// { 1, 1.0f, 0.5f, 2.0f, 0.013f, 0.1f };
	NoiseMap moistMap;// { 2, 1.0f, 0.5f, 2.0f, 0.024f, 1.137f };

	const BiomeType biomeTable[2][2]{
		//Wet          //dry
		{BiomeType::ice, BiomeType::tundra},    //Cold
		{BiomeType::woodland, BiomeType::desert} //Hot
	};

	//const glm::vec2 centerPoints[2][2]{
	//	glm::vec2{COLDLIMIT / 2.0f, 1 - WETLIMIT / 2.0f}, glm::vec2{1 - COLDLIMIT / 2.0f, 1 - WETLIMIT / 2.0f},
	//	glm::vec2{COLDLIMIT / 2.0f, WETLIMIT / 2.0f}, glm::vec2{1 - COLDLIMIT / 2.0f, WETLIMIT / 2.0f }
	//};

	const glm::vec2 centerPoints[2][2]{
		glm::vec2{0.25f, 0.25f}, glm::vec2{0.75f, 0.25f},
		glm::vec2{0.75f, 0.25f}, glm::vec2{0.75f, 0.75f}
	};

	std::vector<std::pair<glm::vec2, BiomeType>> biomepairs;


	std::vector<std::pair<float, BiomeType>> findNearestCentroids(const glm::vec2& p) const {
		std::vector<std::pair<glm::vec2, BiomeType>> nearest;
		for (int i = 0; i < 2; ++i) {
			for (int j = 0; j < 2; ++j) {
				nearest.push_back(std::make_pair(centerPoints[i][j], biomeTable[i][j]));
			}
		}
		std::sort(nearest.begin(), nearest.end(), [&p](const std::pair<glm::vec2, BiomeType>& a, const std::pair<glm::vec2, BiomeType>& b) { return glm::distance(p, a.first) < glm::distance(p, b.first); });

		auto coefs = CartesianToBarycentricCoordinates(p.x, p.y, nearest[0].first, nearest[1].first, nearest[2].first);
		std::vector<std::pair<float, BiomeType>> result;
		//if (coefs.x < 0 || coefs.y < 0 || coefs.z < 0)
		//{
		//	//float d = glm::distance(nearest[0].first, nearest[1].first);
		//	//float c1 = glm::distance(p, nearest[0].first) / d;
		//	result.push_back(std::make_pair(1.0f, nearest[0].second));
		//}
		//else {
			result.push_back(std::make_pair(coefs.x, nearest[0].second));
			result.push_back(std::make_pair(coefs.y, nearest[1].second));
			result.push_back(std::make_pair(coefs.z, nearest[2].second));
		//}



		return result;
		//for (auto p0 : nearest) {
		//	std::cout << "dist: " << glm::distance(p, p0.first) << " ";
		//}
		//std::cout << std::endl;
		//return coefs;
	}

	glm::vec3 CartesianToBarycentricCoordinates(float x, float y, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3) const {
		float x1 = p1.x, x2 = p2.x, x3 = p3.x, y1 = p1.y, y2 = p2.y, y3 = p3.y;

		float A2 = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
		glm::mat3 V{
			x2 * y3 - x3 * y2, y2 - y3, x3 - x2,
			x3 * y1 - x1 * y3, y3 - y1, x1 - x3,
			x1 * y2 - x2 * y1, y1 - y2, x2 - x1
		};
		glm::vec3 B{ 1, x,y };
	
		auto coords = 1 / A2 * glm::transpose(V)* B;
		//std::cout << coords.x << " " << coords.y << " " << coords.z << '\n';

		return coords;
	}

	glm::vec4 computeWeights(float x0, float z0, float r = 1.0f, int samples = 10) const {
		//int samples = 100;
		int counter[4] = {};

		float pi = 3.14f;
		float stepsize = 2 * pi / samples;
		//float r = 1.0f;

		//for (float a = 0.0; a < 2.0 * M_PI + 0.0001; a += 2.0 * M_PI / aSlices)

		for (float a = 0.0f; a <= 2 * pi + 0.0001; a += stepsize) {
			float x = x0 + r * std::cosf(a);
			float z = z0 + r * std::sinf(a);
			BiomeType sampleBiome = getBiomeAtPosition(x, z);
			counter[static_cast<int>(sampleBiome)]++;
		}
		//std::cout << "weigts in func " << counter[0] << " " << counter[1] << " " << counter[2] << " " << counter[3] << '\n';
		float w1 = static_cast<float>(counter[0]) / static_cast<float>(samples);
		float w2 = static_cast<float>(counter[1]) / static_cast<float>(samples);
		float w3 = static_cast<float>(counter[2]) / static_cast<float>(samples);
		float w4 = static_cast<float>(counter[3]) / static_cast<float>(samples);

		return glm::vec4{ w1, w2, w3, w4 };
	}
};