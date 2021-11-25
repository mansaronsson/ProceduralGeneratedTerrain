#pragma once
#include <vector>
#include "Model.h"
#include <glm/gtc/noise.hpp>
#include "BoundingBox.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>

enum chunkChecker {
	inside, up, down, left, right
};

class ChunkHandler {
public:
	/// <summary>
	/// Create a chunk handler 
	/// </summary>
	/// <param name="nrVertices">number of vertecies per chunk</param>
	/// <param name="spacing">distance between vertices</param>
	/// <param name="yscale">how much to scale in the y direction</param>
	ChunkHandler(size_t nrVertices, float spacing, float yscale);


	void draw() {
		for (auto chunk : chunks)	
		{
			chunk->draw();
		}

	}

	void drawBoundingBox() {
		for (auto chunk : chunks) {
			chunk->drawBoundingBox();
		}
	}
	/// <summary>
	/// Check if the camera is inside a chunk on x,z plane 
	/// </summary>
	/// <param name="camPos"></param>
	void checkChunk(const glm::vec3& camPos);

	/// <summary>
	/// Update chunk the camera is currently over ie. get center chunk
	/// </summary>
	/// <param name="ch">what chunk direction did we move into</param>
	void updateCurrentChunk(chunkChecker ch);

private:
	//using enum chunkChecker;
	class Chunk {
	public:
		Chunk(size_t _size, float xpos, float zpos, float _spacing, float yscale, float frequency) : size{ _size } {
			std::vector<Vertex> vertices;
			std::vector<unsigned int> indices;
			vertices.reserve(size * size);
			indices.reserve(6 * (size - 2) * (size - 2) + 3 * size * 4 - 18); // se notes in lecture 6 

			float minY = std::numeric_limits<float>::max();
			float maxY = std::numeric_limits<float>::min();


			for (int depth = 0; depth < size; ++depth)
			{
				for (int width = 0; width < size; ++width) {
					float x = xpos + width * _spacing;
					float z = zpos + depth * _spacing;
					float noiseY = glm::perlin(glm::vec3(x / frequency, z / frequency, 0.1f)); // last component seed 
					noiseY *= yscale;

					glm::vec3 pos{ x, noiseY, z};
					vertices.push_back({ pos });

					minY = minY > noiseY ? noiseY : minY;
					maxY = maxY < noiseY ? noiseY : maxY;


					unsigned int i1, i2, i3, i4;
					if (width < size - 1 &&  depth < size -1) {
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
					glm::vec3 normal{ 0.0f,0.0f,0.0f };

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

			glm::vec3 p1{ minX, maxY, minZ }, p2{ maxX, maxY, minZ }, p3{ maxX, maxY, maxZ }, p4{ minX, maxY, maxZ },
				p5{ minX, minY, minZ }, p6{ maxX, minY, minZ }, p7{ maxX, minY, maxZ }, p8{ minX, minY, maxZ };
			//
			//std::cout << glm::to_string(p1) << " " << glm::to_string(p2) << " " << glm::to_string(p3) << '\n'
			//	<< glm::to_string(p4) << " " << glm::to_string(p5) << " " << glm::to_string(p6) << '\n'
			//	<< glm::to_string(p7) << " " << glm::to_string(p8) << '\n' << '\n';

			boundingBox = BoundingBox{ p1, p2, p3, p4, p5, p6, p7, p8 };
		}

		void draw() {
			mesh.draw(GL_TRIANGLES);
		}
		void drawBoundingBox() {
			boundingBox.draw();
		}
		
		chunkChecker isInside(const glm::vec3& pos);

	private:
		unsigned int index(int w, int d) {
			return w + size * d;

		}
		const size_t size;

		Mesh mesh;
		BoundingBox boundingBox;
	};
	/*End of chunk class*/

	unsigned int index(int col, int row) {
		return col + chunkGridSize * row;
	}

	const int chunkGridSize = 3;
	std::vector<Chunk*> chunks;
	Chunk* currentChunk;
};