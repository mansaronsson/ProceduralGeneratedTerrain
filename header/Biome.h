#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <string>
#include "Mesh.h"
#include <iostream>

const std::string temperatureString[2]{ "cold", "hot" };
const std::string moistureString[2]{ "wet", "dry" };
const std::string biomeString[4]{ "ice", "tundra", " woodland", "desert" };
const glm::vec3 colors[4]{ glm::vec3{0.3f, 1.0f, 1.0f}, glm::vec3{0.3f, 1.0f, 0.7f}, glm::vec3{0.15f, 0.5f, 0.08f}, glm::vec3{1.0f, 0.7f, 0.1f} };
const glm::vec3 tempcolors[2]{ glm::vec3{0.0f, 0.5f, 1.0f}, glm::vec3{1.0f, 0.1f, 0.0f} };
const glm::vec3 moistcolors[2]{ glm::vec3{0.0f, 0.5f, 1.0f}, glm::vec3{0.2f, 1.0f, 0.2f} };


/* noise map testing */
struct NoiseMap {
#include <glm/gtc/noise.hpp>
	int octaves;
	float amplitude;
	float gain;
	float lacunarity;
	float frequency;
	float seed;

	float evaluate(float x, float z) {
		float noiseSum = 0.0f;

		for (int i = 0; i < octaves; ++i) {
			noiseSum += amplitude * glm::perlin(glm::vec3((x + 1) * frequency, (z + 1) * frequency, seed));
			frequency *= lacunarity;
			amplitude *= gain;
		}

		return noiseSum;
	}
};

enum BiomeType {
	ice, tundra, woodland, desert
};

class Biome {
public:
	Biome(float hot = 0.0f, float wet = 0.0f) : heatMap{ NoiseMap{ 1, 1.0f, 0.5f, 2.0f, 0.013f, 0.1f } }, moistMap{ NoiseMap{2, 1.0f, 0.5f, 2.0f, 0.024f, 1.137f} }, coldValue{ hot }, wetValue{ wet }{}

	void getColorFromBiome(Vertex& v, float x, float z) {

		float heatValue = heatMap.evaluate(x, z);
		float moistValue = moistMap.evaluate(x, z);
		std::cout << heatValue << " " << moistValue << '\n';
		Temperature heat = getTemperatureAtValue(heatValue);
		Moisture moist = getMoistureAtValue(moistValue);

		BiomeType biome = computeBiome(heat, moist);

		v.biomecolor = colors[(int)biome];
		v.heatcolor = tempcolors[(int)heat];
		v.moistcolor = moistcolors[(int)moist];
	}

	BiomeType getBiomeAtPosition(float x, float z) {
		float heatValue = heatMap.evaluate(x, z);
		float moistValue = moistMap.evaluate(x, z);
		//std::cout << heatValue << " " << moistValue << '\n';
		Temperature heat = getTemperatureAtValue(heatValue);
		Moisture moist = getMoistureAtValue(moistValue);

		return computeBiome(heat, moist);
	}

private:
	//enum BiomeType {
	//	ice, tundra, woodland, desert
	//};
	enum Temperature
	{
		cold, hot
	};

	enum class Moisture {
		wet, dry
	};

	Temperature getTemperatureAtValue(float val) {
		if (val < coldValue)
			return Temperature::cold;
		else
			return Temperature::hot;
	}
	Moisture getMoistureAtValue(float val) {
		if (val < wetValue)
			return Moisture::wet;
		else
			return Moisture::dry;
	}

	BiomeType computeBiome(Temperature temp, Moisture moist) {
		return biomeTable[static_cast<int>(temp)][static_cast<int>(moist)];
	}

	const float coldValue, wetValue;

	NoiseMap heatMap;// { 1, 1.0f, 0.5f, 2.0f, 0.013f, 0.1f };
	NoiseMap moistMap;// { 2, 1.0f, 0.5f, 2.0f, 0.024f, 1.137f };

	const BiomeType biomeTable[2][2] {
		//Cold          //Hot
		BiomeType::ice, BiomeType::tundra,     //Wet
		BiomeType::woodland, BiomeType::desert //Dry
	};

};